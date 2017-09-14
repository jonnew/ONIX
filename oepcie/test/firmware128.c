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

// Params
const size_t num_chan = 128;
const size_t run_time_sec = 10;
const size_t samp_per_chan_per_block = 1000;
const size_t fs_hz = 30e3;

// Should we enter the data production loop
volatile int running = 0;
volatile int data_gen_complete = 0;

// Data acq. params
size_t samp_per_block;
size_t num_blocks;
//const size_t sample_time_us = (1e6 * samp_per_chan_per_block ) / fs_hz;

// FIFO file path and desciptor handles
const char *config_path = "/tmp/rat128_config";
const char *sig_path = "/tmp/rat128_signal";
const char *data_path = "/tmp/rat128_data";
int config_fd = -1;
int data_fd = -1;
int sig_fd = -1;

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

void *data_loop(void *vargp)
{
    // Data generation parameters
    samp_per_block = num_chan * samp_per_chan_per_block;
    num_blocks = (run_time_sec * fs_hz) / samp_per_chan_per_block;

    // Set data pipe capacity
    // Sample number, LFP data, ...
    fcntl(data_fd, F_SETPIPE_SZ, sizeof(uint64_t) + samp_per_block * sizeof(int16_t));

    // Start data generation loop
    int i = 0;
    uint64_t sample = 0;

    while (i < num_blocks) {

        if (!running) {
            usleep(10000);
            continue;
        }

        // Generate data block
        // 1. Sample number
        write(data_fd, &sample, 8);

        // 2. LFP data
        int16_t lfp_block[samp_per_block];
        int j;
        for (j = 0; j < samp_per_block; j++)
            lfp_block[j] = (int16_t)randn(0, 500);

        size_t rc
            = write(data_fd, lfp_block, samp_per_block * sizeof(int16_t));
        printf("Write %zu bytes\n", rc);

        // Increment sample count
        sample += samp_per_chan_per_block;
        i++;
    }

    data_gen_complete = 1;
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
    while (!data_gen_complete) {

        // Slow loop
        usleep(100000);

        // Poll configuration registers
        // NB: It is very important to 0-initalize the register values here
        // because (I think) they are cast to void * in read_config and
        // this is bad for some reason
        int32_t dev_id;
        read_config(config_fd, OE_CONFDEVID, &dev_id, 4);
        // printf("Dev ID: %d\n", dev_id);

        uint32_t reg_addr;
        read_config(config_fd, OE_CONFADDR, &reg_addr, 4);
        // printf("Reg. Addr: %u\n", reg_addr);

        uint32_t reg_val;
        read_config(config_fd, OE_CONFVALUE, &reg_val, 4);
        // printf("Reg. val.: %u\n", reg_val);

        uint8_t reg_rw;
        read_config(config_fd, OE_CONFRW, &reg_rw, 1);
        // printf("Reg. RW: %hhu\n", reg_rw);

        uint8_t trig_value;
        read_config(config_fd, OE_CONFTRIG, &trig_value, 1);
        // printf("Trig val.: %hhu\n", trig_value);

        if (trig_value && !reg_rw) { // Read operation requested by host

            printf("Register read requested.\n");

            // TODO: Go get the value. For now, we are just mirror the value
            // in reg_val
            uint32_t read_reg = reg_val;

            // Untrigger
            trig_value = 0x00;
            write_config(config_fd, OE_CONFTRIG, &trig_value, 1);

            // Send the result back
            send_data_signal(
                sig_fd, OE_CONFIGRACK, &read_reg, sizeof(read_reg));

        } else if (trig_value && reg_rw) { // write operation requested by host

            printf("Register write requested.\n");

            // TODO: Use the value register to program device register

            // Untrigger
            trig_value = 0x00;
            write_config(config_fd, OE_CONFTRIG, &trig_value, 1);

            // Are we targetting master device?
            int header_requested = 0;
            if (dev_id == OEPCIEMASTER) { // Thats me!
                switch (reg_addr) {

                    case OEPCIEMASTER_HEADER: // Header request
                        header_requested = 1;
                        break;
                    case OEPCIEMASTER_RUNNING: // Set running
                        running = reg_val;
                        break;
                }
            }

            // Send read acknowledge back to host
            send_msg_signal(sig_fd, OE_CONFIGWACK);

            // Push the header into the singal FIFO
            if (header_requested) {

                send_msg_signal(sig_fd, OE_HEADERSTART);

                // Two intan chips and an IMU
                oe_device_t this_dev;
                this_dev.id = RHD2064;
                this_dev.read_offset = 0;
                this_dev.read_size = 66 * sizeof(uint16_t);
                this_dev.write_offset = 0;
                this_dev.write_size = 0;
                send_data_signal(sig_fd, OE_DEVICEINST, &this_dev, sizeof(oe_device_t));

                this_dev.id = RHD2064;
                this_dev.read_offset = 66 * sizeof(uint16_t);
                this_dev.read_size = 66 * sizeof(uint16_t);
                this_dev.write_offset = 0;
                this_dev.write_size = 0;
                send_data_signal(sig_fd, OE_DEVICEINST, &this_dev, sizeof(oe_device_t));

                this_dev.id = MPU950;
                this_dev.read_offset = 2 * (66 * sizeof(uint16_t));
                this_dev.read_size = 4 * 6;
                this_dev.write_offset = 0;
                this_dev.write_size = 0;
                send_data_signal(sig_fd, OE_DEVICEINST, &this_dev, sizeof(oe_device_t));
                
                send_msg_signal(sig_fd, OE_HEADEREND);
            }
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
