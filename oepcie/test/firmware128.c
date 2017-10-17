#define _GNU_SOURCE // fcntl

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
const size_t fifo_buffer_samples = 1000;

// Config registers
volatile oe_reg_val_t running = 0;
const oe_reg_val_t sys_clock_hz = 100e6;
oe_reg_val_t clock_m = 30;
oe_reg_val_t clock_d = 100;
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
        .read_offset = OE_RFRAMEHEADERSZ,
        .read_size = 66 * sizeof(uint16_t),
        .write_offset = OE_WFRAMEHEADERSZ,
        .write_size = 0},
       {.id = OE_RHD2164,
        .read_offset = OE_RFRAMEHEADERSZ + 66 * sizeof(uint16_t),
        .read_size = 66 * sizeof(uint16_t),
        .write_offset = OE_WFRAMEHEADERSZ,
        .write_size = 0},
       {.id = OE_MPU9250,
        .read_offset = OE_RFRAMEHEADERSZ + 2 * (66 * sizeof(uint16_t)),
        .read_size = 4 * 6,
        .write_offset = OE_WFRAMEHEADERSZ,
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

uint32_t get_read_frame_size(oe_device_t *dev_map, size_t n_devs)
{
    int i;
    int max_idx = 0;
    for (i = 0; i < n_devs; i++) {

        oe_device_t cur_dev = dev_map[i];
        if (cur_dev.read_offset > dev_map[max_idx].read_offset)
            max_idx = i;
    }

    size_t read_frame_size = dev_map[max_idx].read_offset + dev_map[max_idx].read_size;
    read_frame_size += read_frame_size % 32; // padding

    return read_frame_size;
}

uint32_t get_write_frame_size(oe_device_t *dev_map, size_t n_devs)
{
    int i;
    int max_idx = 0;
    for (i = 0; i < n_devs; i++) {

        oe_device_t cur_dev = dev_map[i];
        if (cur_dev.write_offset > dev_map[max_idx].write_offset)
            max_idx = i;
    }

    size_t write_frame_size = dev_map[max_idx].write_offset + dev_map[max_idx].write_size;
    write_frame_size += write_frame_size % 32; // padding

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
    clock_m = 30;
    clock_d = 100;
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
    uint32_t num_dev = sizeof(my_devices) / sizeof(oe_device_t);
    send_data_signal(sig_fd, DEVICEMAPACK, &num_dev, sizeof(num_dev));

    uint32_t r_size = get_read_frame_size(my_devices, num_dev);
    send_data_signal(sig_fd, FRAMERSIZE, &r_size, sizeof(r_size));

    uint32_t w_size = get_write_frame_size(my_devices, num_dev);
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

    // Number of devices
    const size_t num_dev = sizeof(my_devices) / sizeof(oe_device_t);

    // Sample number + total bytes in samples
    // NB: This fragile because it assumes that the devices are in order of
    // read offsets
    const size_t sample_size = my_devices[num_dev - 1].read_offset
                               + my_devices[num_dev - 1].read_size;

    // Set data pipe capacity
    // Sample number, LFP data, ...
    fcntl(data_fd, F_SETPIPE_SZ, sample_size * fifo_buffer_samples);

    while (!quit) {
        if (running) {

            usleep(1e6 / fs_hz); // Simulate finite sampling time

            // Buffer for data
            uint8_t sample[sample_size];

            // Leading sample_tick
            *(uint64_t *)sample = sample_tick;

            // Generate sample (frame)
            int j;
            for (j = OE_RFRAMEHEADERSZ; j < sample_size; j += 2)
                *(uint16_t *)(sample + j) = (uint16_t)randn(32768, 50);

            size_t rc = write(data_fd, sample, sample_size);
            printf("Write %zu bytes\n", rc);

            // Increment sample count
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

    // Join data and singal threads
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
