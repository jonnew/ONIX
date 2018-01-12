#define _GNU_SOURCE // fcntl

#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../liboepcie/oepcie.h"
#include "testfunc.h"

// Data acq. params
const size_t approx_frame_size = 500;
const size_t fifo_frame_capacity = 1000;

// Config registers
volatile oe_reg_val_t running = 0;
const oe_reg_val_t sys_clock_hz = 100e6;
oe_reg_val_t clock_m = 1;
oe_reg_val_t clock_d = 10000; // These vals give 10 kHz
volatile oe_reg_val_t fs_hz;

// Global state
volatile uint64_t sample_tick = 0;

// Thread control
volatile int quit = 0;

// FIFO file path and desciptor handles
const char *config_path = "/tmp/rat128_config";
const char *sig_path = "/tmp/rat128_signal";
const char *data_path = "/tmp/rat128_read";
int config_fd = -1;
int data_fd = -1;
int sig_fd = -1;

// These enums are REDEFINED here (from oepcie.c) because they are static there.
typedef enum oe_signal {
    NULLSIG             = (1u << 0),
    CONFIGWACK          = (1u << 1), // Configuration write-acknowledgement
    CONFIGWNACK         = (1u << 2), // Configuration no-write-acknowledgement
    CONFIGRACK          = (1u << 3), // Configuration read-acknowledgement
    CONFIGRNACK         = (1u << 4), // Configuration no-read-acknowledgement
    DEVICEMAPACK        = (1u << 5), // Device map start acnknowledgement
    FRAMERSIZE          = (1u << 6), // Frame read size in bytes
    FRAMEWSIZE          = (1u << 7), // Frame write size in bytes
    DEVICEINST          = (1u << 8), // Deivce map instance
} oe_signal_t;

// Configuration file offsets
typedef enum oe_conf_reg_off {

    // Register R/W interface
    CONFDEVIDOFFSET     = 0,   // Configuration device id register byte offset
    CONFADDROFFSET      = 4,   // Configuration register address register byte offset
    CONFVALUEOFFSET     = 8,   // Configuration register value register byte offset
    CONFRWOFFSET        = 12,  // Configuration register read/write register byte offset
    CONFTRIGOFFSET      = 16,  // Configuration read/write trigger register byte offset

    // Global configuration
    CONFRUNNINGOFFSET   = 20,  // Configuration run hardware register byte offset
    CONFRESETOFFSET     = 24,  // Configuration reset hardware register byte offset
    CONFSYSCLKHZOFFSET  = 28,  // Configuration base clock frequency register byte offset
    CONFFSCLKHZOFFSET   = 32,  // Configuration frame clock frequency register byte offset
    CONFFSCLKMOFFSET    = 36,  // Configuration run hardware register byte offset
    CONFFSCLKDOFFSET    = 40,  // Configuration run hardware register byte offset
} oe_conf_reg_off_t;

// Devices handled by this firmware
static oe_device_t my_devices[]
    = {{.id = OE_RHD2164,
        .read_size = 67 * sizeof(uint16_t),
        .write_size = 0},
       {.id = OE_RHD2164,
        .read_size = 67 * sizeof(uint16_t),
        .write_size = 0},
       {.id = OE_MPU9250,
        .read_size = 9 * sizeof(uint16_t),
        .write_size = 0}};

// Normal distribution
double randn(double mu, double sigma)
{
    double U1, U2, W, mult;
    static double X1, X2;
    static int call = 0;

    if (call == 1) {
        call = !call;
        return (mu + sigma * (double)X2);
    }

    do {
        U1 = -1 + ((double)rand() / RAND_MAX) * 2;
        U2 = -1 + ((double)rand() / RAND_MAX) * 2;
        W = pow(U1, 2) + pow(U2, 2);
    } while (W >= 1 || W == 0);

    mult = sqrt((-2 * log(W)) / W);
    X1 = U1 * mult;
    X2 = U2 * mult;

    call = !call;

    return (mu + sigma * (double)X1);
}

uint32_t get_read_frame_size(oe_device_t *dev_map, int *devs, size_t n_devs)
{
    int i;
    size_t read_frame_size = OE_RFRAMEHEADERSZ + sizeof(uint32_t) * n_devs;
    for (i = 0; i < n_devs; i++) {

        oe_device_t cur_dev = dev_map[devs[i]];
        read_frame_size += cur_dev.read_size;
    }

    read_frame_size += read_frame_size % 4; // padding

    return read_frame_size;
}

uint32_t get_write_frame_size(oe_device_t *dev_map, int *devs, size_t n_devs)
{
    int i;
    size_t write_frame_size = OE_RFRAMEHEADERSZ + sizeof(uint32_t) * n_devs;
    for (i = 0; i < n_devs; i++) {

        oe_device_t cur_dev = dev_map[devs[i]];
        write_frame_size += cur_dev.read_size;
    }

    write_frame_size += write_frame_size % 4; // padding

    return write_frame_size;
}

size_t div_clock(size_t base_freq_hz, uint32_t M, uint32_t D)
{
    if (M >= D)
        return base_freq_hz;
    else
        return (M * base_freq_hz)/D;
}

int read_config(int config_fd, oe_conf_reg_off_t offset, void *result, size_t size)
{
    lseek(config_fd, offset, SEEK_SET);
    read(config_fd, result, size);
    return 0;
}

int write_config(int config_fd, oe_conf_reg_off_t offset, void *value, size_t size)
{
    lseek(config_fd, offset, SEEK_SET);
    write(config_fd, value, size);
    return 0;
}

void generate_default_config(int config_fd)
{
    // Default config
    running = 0;
    fs_hz = div_clock(sys_clock_hz, clock_m, clock_d);
    sample_tick = 0;

    // Just put a bunch of 0s in there
    lseek(config_fd, 0, SEEK_SET);
    oe_reg_val_t regs[100] = {0};
    write(config_fd, regs, 100 * sizeof(oe_reg_val_t));

    // Write defaults for registers that need it
    write_config(config_fd, CONFRUNNINGOFFSET, (void *)&running, sizeof(oe_reg_val_t));
    write_config(config_fd, CONFSYSCLKHZOFFSET, (void *)&sys_clock_hz, sizeof(oe_reg_val_t));
    write_config(config_fd, CONFFSCLKHZOFFSET, (void *)&fs_hz, sizeof(oe_reg_val_t));
    write_config(config_fd, CONFFSCLKMOFFSET, (void *)&clock_m, sizeof(oe_reg_val_t));
    write_config(config_fd, CONFFSCLKDOFFSET, (void *)&clock_d, sizeof(oe_reg_val_t));
}

int send_msg_signal(int sig_fd, oe_signal_t type)
{
    // Src and dst buffers
    // COBS data, 1 overhead byte + 0x0 delimiter
    uint8_t dst[sizeof(oe_signal_t) + 2] = {0};

    oe_cobs_stuff(dst, (uint8_t *)&type, sizeof(type));
    write(sig_fd, dst, sizeof(dst));

    return 0;
}

int send_data_signal(int sig_fd, oe_signal_t type, void *data, size_t n)
{
    size_t packet_size = sizeof(oe_signal_t) + n;

    // Make sure packet_size < 254
    if (packet_size > 254)
        return -1;

    // Src and dst buffers
    uint8_t src[packet_size];
    uint8_t dst[256] = {0}; // Maximal packet size including delimeter

    // Concatenate signal type and data
    memcpy(src, &type, sizeof(type));
    if (n > 0 && data != NULL)
        memcpy(src + sizeof(type), data, n);

    // Create COBs packet with overhead byte
    oe_cobs_stuff(dst, src, packet_size);

    // COBS data, 1 overhead byte + 0x0 delimiter
    write(sig_fd, dst, packet_size + 2);

    return 0;
}

void send_device_map(int sig_fd)
{
    int dev_idxs[] = { 0, 1, 2 };
    uint32_t num_dev = sizeof(my_devices) / sizeof(oe_device_t);
    send_data_signal(sig_fd, DEVICEMAPACK, &num_dev, sizeof(num_dev));

    uint32_t r_size = get_read_frame_size(my_devices, dev_idxs, num_dev);
    send_data_signal(sig_fd, FRAMERSIZE, &r_size, sizeof(r_size));

    uint32_t w_size = get_write_frame_size(my_devices, dev_idxs, num_dev);
    send_data_signal(sig_fd, FRAMEWSIZE, &w_size, sizeof(w_size));

    // Loop through devices
    int i;
    for (i = 0; i < sizeof(my_devices)/sizeof(oe_device_t); i++)
        send_data_signal(sig_fd, DEVICEINST, &my_devices[i], sizeof(oe_device_t));
}

void *data_loop(void *vargp)
{
    // Initial clock conifig
    fs_hz = div_clock(sys_clock_hz, clock_m, clock_d);

    // Set data pipe capacity
    // Sample number, LFP data, ...
    fcntl(data_fd, F_SETPIPE_SZ, approx_frame_size * fifo_frame_capacity);

    //const int fudge_factor = 1;

    while (!quit) {
        if (running) {

            //usleep(1e6 / fs_hz - fudge_factor); // Simulate finite sampling time

            // Raw frame
            uint8_t * frame;
            uint16_t num_devs;
            size_t data_block_size = 0;
            size_t frame_size;
            if (sample_tick % 100 == 0) { // Include IMU data
                num_devs = 3;
                int dev_idxs[] = {0, 1, 2};
                int i;
                for (i = 0; i < num_devs; i++)
                    data_block_size += my_devices[dev_idxs[i]].read_size;

                frame_size
                    = get_read_frame_size(my_devices, dev_idxs, num_devs);
                frame = malloc(frame_size);

                // Device indicies
                *(((uint32_t *)(frame + OE_RFRAMEHEADERSZ)) + 0) = 0;
                *(((uint32_t *)(frame + OE_RFRAMEHEADERSZ)) + 1) = 1;
                *(((uint32_t *)(frame + OE_RFRAMEHEADERSZ)) + 2) = 2;

            } else {
                num_devs = 2;
                int dev_idxs[] = {0, 1};
                int i;
                for (i = 0; i < num_devs; i++)
                    data_block_size += my_devices[dev_idxs[i]].read_size;

                frame_size
                    = get_read_frame_size(my_devices, dev_idxs, num_devs);
                frame = malloc(frame_size);

                // Device indicies
                *(((uint32_t *)(frame + OE_RFRAMEHEADERSZ)) + 0) = 0;
                *(((uint32_t *)(frame + OE_RFRAMEHEADERSZ)) + 1) = 1;
            }

            // Frame header
            *(uint64_t *)frame = sample_tick;                      // Sample number
            *((uint16_t *)(frame + OE_RFRAMENDEVOFF)) = num_devs;  // Num devices
            *(frame + OE_RFRAMENERROFF) = 0;                       // Error

            // Where does the data block start and end
            uint8_t *data_ptr
                = frame + (OE_RFRAMEHEADERSZ + num_devs * sizeof(uint32_t));
            uint8_t *data_end = data_ptr + data_block_size;

            // Generate frame
            // TODO: Generate the proper raw type for the device
            while (data_ptr < data_end) {
                *(uint16_t *)(data_ptr) = sample_tick % 65535;
                data_ptr += 2;
                if (data_ptr >= data_end)
                    break;
                *(uint16_t *)(data_ptr) = 65535 - (sample_tick % 65535);
                data_ptr += 2;
                if (data_ptr >= data_end)
                    break;
                *(uint16_t *)(data_ptr) = 65535 * (sample_tick % 2);
                data_ptr += 2;
                if (data_ptr >= data_end)
                    break;
            }

            size_t rc = write(data_fd, frame, frame_size);
            assert(rc == frame_size && "Incomplete write.");
            //printf("Write %zu bytes\n", rc);

            // Increment frame count
            sample_tick += 1;

        } else {
            usleep(1000); // Prevent CPU tacking
        }
    }

    return NULL;
}

int main()
{
    // Start fresh
    unlink(sig_path);
    unlink(data_path);
    unlink(config_path);

    // Async streams are similar to pipes
    mkfifo(data_path, 0666);
    mkfifo(sig_path, 0666);

    // Open FIFOs for write only and config file for read/write

    // NB: This must be done first to prevent race condition -- host will try
    // to read noexistant file if fifos are opened before this. This should not
    // be an issue with xillybus.
    config_fd = open(config_path, O_RDWR | O_CREAT, 0666);

    // NB: Must respect this ordering when opening files in host or the two
    // programs will deadlock. The ordering is hidden in oe_init_ctx(), so its
    // up to firmware to do it correctly. I need to make sure this won't be an
    // issue when using xillybus
    data_fd = open(data_path, O_WRONLY);
    sig_fd = open(sig_path, O_WRONLY);

    // Generate data thread and continue here config/signal handling in parallel
    pthread_t tid;
    pthread_create(&tid, NULL, data_loop, NULL);

reset:

    // Reset run state by generating default configuration
    generate_default_config(config_fd); // Generate config file content

    // Send device map
    send_device_map(sig_fd);

    // Config loop
    while (!quit) {

        // Slow loop. In FPGA, this would be purely async.
        usleep(1000);

        // --  Global registers --
        oe_reg_val_t run;
        read_config(config_fd, CONFRUNNINGOFFSET, &run, 4);
        running = run;

        oe_reg_val_t reset;
        read_config(config_fd, CONFRESETOFFSET, &reset, 4);
        if (reset) {
            reset = 0; // untrigger
            write_config(config_fd, CONFRESETOFFSET, &reset, 4);
            goto reset;
        }

        // Sample rate
        read_config(config_fd, CONFFSCLKMOFFSET, &clock_m, 4);
        read_config(config_fd, CONFFSCLKDOFFSET, &clock_d, 4);
        fs_hz = div_clock(sys_clock_hz, clock_m, clock_d);
        write_config(config_fd, CONFFSCLKHZOFFSET, (void *)&fs_hz, 4);

        // -- Device configuration Interface --
        // Poll configuration registers
        // NB: It is very important to 0-initalize the register values here
        // because (I think) they are cast to void * in read_config and
        // this is bad for some reason
        oe_reg_val_t dev_id;
        read_config(config_fd, CONFDEVIDOFFSET, &dev_id, 4);
        // printf("Dev ID: %d\n", dev_id);

        oe_reg_val_t reg_addr;
        read_config(config_fd, CONFADDROFFSET, &reg_addr, 4);
        // printf("Reg. Addr: %u\n", reg_addr);

        oe_reg_val_t reg_val;
        read_config(config_fd, CONFVALUEOFFSET, &reg_val, 4);
        // printf("Reg. val.: %u\n", reg_val);

        oe_reg_val_t reg_rw;
        read_config(config_fd, CONFRWOFFSET, &reg_rw, 1);
        // printf("Reg. RW: %hhu\n", reg_rw);

        oe_reg_val_t trig_value;
        read_config(config_fd, CONFTRIGOFFSET, &trig_value, 1);
        // printf("Trig val.: %hhu\n", trig_value);

        if (trig_value && !reg_rw) { // Read operation requested by host

            printf("Register read requested.\n");

            // TODO: Go get the value. For now, we are just mirror
            // the value in reg_val
            uint32_t read_reg = reg_val;

            // Send the result back
            send_data_signal(
                sig_fd, CONFIGRACK, &read_reg, sizeof(read_reg));

            // Untrigger
            trig_value = 0x00;
            write_config(config_fd, CONFTRIGOFFSET, &trig_value, 1);

        } else if (trig_value && reg_rw) { // write operation requested by host

            // TODO: Use the value register to program device register
            // (not impelemented here)

            // Send read acknowledge back to host
            send_msg_signal(sig_fd, CONFIGWACK);

            // Untrigger
            trig_value = 0x00;
            write_config(config_fd, CONFTRIGOFFSET, &trig_value, 1);
        }
    }

    // Join data and signal threads
    pthread_join(tid, NULL);

    // Close pipes/files
    close(sig_fd);
    close(data_fd);
    close(config_fd);

    // Delete files
    unlink(sig_path);
    unlink(data_path);
    unlink(config_path);

    return 0;
}
