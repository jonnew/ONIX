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

#include "../liboepcie/oedevice.h"
#include "../liboepcie/oepcie.h"
#include "testfunc.h"

// Data acq. params
const size_t base_clock_hz= 100e6;
const size_t fifo_buffer_samples = 100; 

// Config registers
volatile uint32_t clock_m = 30;
volatile uint32_t clock_d = 100;
volatile uint32_t fs_hz;
volatile int acquiring = 0;
volatile int quit = 0;

// FIFO file path and desciptor handles
const char *config_path = "/tmp/rat128_config";
const char *sig_path = "/tmp/rat128_signal";
const char *data_path = "/tmp/rat128_data";
int config_fd = -1;
int data_fd = -1;
int sig_fd = -1;

// These enums are REDEFINED here (from oepcie.c) because they are static there.
typedef enum oe_signal {
    NULLSIG        = (1u << 0),
    CONFIGWACK     = (1u << 1), // Configuration write-acknowledgement
    CONFIGWNACK    = (1u << 2), // Configuration no-write-acknowledgement
    CONFIGRACK     = (1u << 3), // Configuration read-acknowledgement
    CONFIGRNACK    = (1u << 4), // Configuration no-read-acknowledgement
    DEVICEINST     = (1u << 5), // Deivce instance
} oe_signal_t;

typedef enum oe_config_reg_offset {
    CONFDEVIDOFFSET        = 0,   // Configuration device id register byte offset
    CONFADDROFFSET         = 4,   // Configuration register address register byte offset
    CONFVALUEOFFSET        = 8,   // Configuration register value register byte offset
    CONFRWOFFSET           = 12,  // Configuration register read/write register byte offset
    CONFTRIGOFFSET         = 13,  // Configuration read/write trigger register byte offset
} oe_config_reg_offset_t;

// Devices handled by this firmware
static oe_device_t my_devices[]
    = {{.id = OE_RHD2064,
        .read_offset = OE_READFRAMEOVERHEAD,
        .read_size = 66 * sizeof(uint16_t),
        .write_offset = 0,
        .write_size = 0},
       {.id = OE_RHD2064,
        .read_offset = OE_READFRAMEOVERHEAD + 66 * sizeof(uint16_t),
        .read_size = 66 * sizeof(uint16_t),
        .write_offset = 0,
        .write_size = 0},
       {.id = OE_MPU9250,
        .read_offset = OE_READFRAMEOVERHEAD + 2 * (66 * sizeof(uint16_t)),
        .read_size = 4 * 6,
        .write_offset = 0,
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

size_t div_clock(size_t base_freq_hz, uint32_t M, uint32_t D) 
{
    if (M >= D)
        return base_freq_hz;
    else
        return (M * base_freq_hz)/D;
}

void generate_config(int config_fd)
{
    // Just put a bunch of 0s in there
    lseek(config_fd, 0, SEEK_SET);
    uint8_t regs[100] = {0};
    write(config_fd, regs, sizeof(regs));
}

int read_config(int config_fd, oe_config_reg_offset_t offset, void *result, size_t size)
{
    lseek(config_fd, offset, SEEK_SET);
    read(config_fd, result, size);
    return 0;
}

int write_config(int config_fd, oe_config_reg_offset_t offset, void *value, size_t size)
{
    lseek(config_fd, offset, SEEK_SET);
    write(config_fd, value, size);
    return 0;
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

void send_header(int sig_fd) {

    // Loop through devices
    int i;
    for (i = 0; i < sizeof(my_devices)/sizeof(oe_device_t); i++)
        send_data_signal(sig_fd, DEVICEINST, &my_devices[i], sizeof(oe_device_t));
}

void *data_loop(void *vargp)
{
    // Initial clock conifig
    fs_hz = div_clock(base_clock_hz, clock_m, clock_d);

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

    // Start data generation loop
    uint64_t sample_tick = 0;

    while (!quit) {

        usleep(1e6 / fs_hz); // Simulate finite sampling time

        if (acquiring) {
        // Buffer for data
        uint8_t sample[sample_size];

        // Leading sample_tick
        *(uint64_t *)sample = sample_tick;

        // Generate sample (frame)
        int j;
        for (j = 8; j < sample_size; j+=2)
            *(uint16_t *)(sample + j) = (uint16_t)randn(32768, 50);

            size_t rc = write(data_fd, sample, sample_size);
            printf("Write %zu bytes\n", rc);
        }

        // Increment sample count
        sample_tick += 1;
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
    generate_config(config_fd); // Generate config file content

    // NB: Must respect this ordering when opening files in host or the two
    // programs will deadlock. The ordering is hidden in oe_init_ctx(), so its
    // up to firmware to do it correctly. I need to make sure this won't be an
    // issue when using xillybus
    data_fd = open(data_path, O_WRONLY);
    sig_fd = open(sig_path, O_WRONLY);

    // Generate data thread and continue here config/signal handling in parallel
    pthread_t tid;
    pthread_create(&tid, NULL, data_loop, NULL);

    // Config loop
    while (!quit) {

        // Slow loop. In FPGA, this would be purely async.
        usleep(100000);

        // Poll configuration registers
        // NB: It is very important to 0-initalize the register values here
        // because (I think) they are cast to void * in read_config and
        // this is bad for some reason
        int32_t dev_id;
        read_config(config_fd, CONFDEVIDOFFSET, &dev_id, 4);
        // printf("Dev ID: %d\n", dev_id);

        uint32_t reg_addr;
        read_config(config_fd, CONFADDROFFSET, &reg_addr, 4);
        // printf("Reg. Addr: %u\n", reg_addr);

        uint32_t reg_val;
        read_config(config_fd, CONFVALUEOFFSET, &reg_val, 4);
        // printf("Reg. val.: %u\n", reg_val);

        uint8_t reg_rw;
        read_config(config_fd, CONFRWOFFSET, &reg_rw, 1);
        // printf("Reg. RW: %hhu\n", reg_rw);

        uint8_t trig_value;
        read_config(config_fd, CONFTRIGOFFSET, &trig_value, 1);
        // printf("Trig val.: %hhu\n", trig_value);

        if (trig_value && !reg_rw) { // Read operation requested by host

            printf("Register read requested.\n");

            // Are we targetting master device?
            if (dev_id == OE_PCIEMASTERDEVIDX) { // Thats me!
                switch (reg_addr) {
                    case OE_PCIEMASTERDEV_HEADER: {
                        // Num dev, dev 0, dev 1, dev 2, ...
                        uint32_t num_dev = sizeof(my_devices) / sizeof(oe_device_t);
                        send_data_signal(sig_fd, CONFIGRACK, &num_dev, sizeof(num_dev));
                        send_header(sig_fd);
                        break;
                    }
                    case OE_PCIEMASTERDEV_RUNNING: {
                        // Num dev, dev 0, dev 1, dev 2, ...
                        uint32_t is_running = acquiring;
                        send_data_signal(sig_fd, CONFIGRACK, &is_running, sizeof(is_running));
                        break;
                    }
                    case OE_PCIEMASTERDEV_RESET: { 
                        uint32_t reset = quit;
                        send_data_signal(sig_fd, CONFIGRACK, &reset, sizeof(reset));
                        break;
                    }
                    case OE_PCIEMASTERDEV_BASECLKHZ: {
                        uint32_t hz = base_clock_hz;
                        send_data_signal(sig_fd, CONFIGRACK, &hz, sizeof(hz));
                        break;
                    }
                    case OE_PCIEMASTERDEV_FSCLKHZ: {
                        uint32_t hz = fs_hz;
                        send_data_signal(sig_fd, CONFIGRACK, &hz, sizeof(hz));
                        break;
                    }
                    case OE_PCIEMASTERDEV_CLKM:{
                        uint32_t m = clock_m;
                        send_data_signal(sig_fd, CONFIGRACK, &m, sizeof(m));
                        break;
                    }
                    case OE_PCIEMASTERDEV_CLKD: {
                        uint32_t d = clock_d;
                        send_data_signal(sig_fd, CONFIGRACK, &d, sizeof(d));
                        break;
                    }
                }

            } else { // Other devices (not implemented here)

                // TODO: Go get the value. For now, we are just mirror
                // the value in reg_val
                uint32_t read_reg = reg_val;

                // Send the result back
                send_data_signal(
                    sig_fd, CONFIGRACK, &read_reg, sizeof(read_reg));
            }

            // Untrigger
            trig_value = 0x00;
            write_config(config_fd, CONFTRIGOFFSET, &trig_value, 1);

        } else if (trig_value && reg_rw) { // write operation requested by host

            printf("Register write requested.\n");

            // Are we targetting master device?
            if (dev_id == OE_PCIEMASTERDEVIDX) { // Thats me!
                switch (reg_addr) {

                    case OE_PCIEMASTERDEV_HEADER:
                        send_msg_signal(sig_fd, CONFIGWNACK); // Send nack since this read only
                        break;
                    case OE_PCIEMASTERDEV_RUNNING: // Set acquiring
                        acquiring = reg_val;
                        send_msg_signal(sig_fd, CONFIGWACK); 
                        break;
                    case OE_PCIEMASTERDEV_RESET: // Set quit
                        quit = reg_val;
                        send_msg_signal(sig_fd, CONFIGWACK);
                        break;
                    case OE_PCIEMASTERDEV_BASECLKHZ:
                        send_msg_signal(sig_fd, CONFIGWNACK); // Send nack since this read only
                        break;
                    case OE_PCIEMASTERDEV_FSCLKHZ:
                        send_msg_signal(sig_fd, CONFIGWNACK); // Send nack since this read only
                        break;
                    case OE_PCIEMASTERDEV_CLKM:
                        clock_m = reg_val; // Set clock multiply
                        fs_hz = div_clock(base_clock_hz, clock_m, clock_d);
                        send_msg_signal(sig_fd, CONFIGWACK);
                        break;
                    case OE_PCIEMASTERDEV_CLKD:
                        clock_d = reg_val; // Set clock divide
                        fs_hz = div_clock(base_clock_hz, clock_m, clock_d);
                        send_msg_signal(sig_fd, CONFIGWACK);
                        break;
                }

            } else { // Other devices (not implemented here)

                // TODO: Use the value register to program device register

                // Send read acknowledge back to host
                send_msg_signal(sig_fd, CONFIGWACK);
            }

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
