#define _GNU_SOURCE

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../liboepcie/oepcieutil.h"

// Params
const size_t num_chan = 128;
const size_t run_time_sec = 10;
const size_t samp_per_chan_per_block = 1000;
const size_t fs_hz = 30e3;

// Reproduced from oepcie.c
typedef enum config_reg_offset {
    CONFIG_DEVICE_IDX   = 0x00,
    CONFIG_REG_ADDR     = 0x04,
    CONFIG_REG_VALUE    = 0x08,
    CONFIG_WRITE_TRIG   = 0x0c,
    CONFIG_READ_VALUE   = 0x10,
    CONFIG_READ_TRIG    = 0x14,
    //CONFIG_WRITE_ACK
    //CONFIG_READ_ACK
} config_reg_offset_t;

typedef enum signal_ack {
    HEADER_READ_ACK,
    HEADER_READ_ACK_E,
    CONFIG_READ_ACK,
    CONFIG_READ_ACK_E,
    CONFIG_WRITE_ACK,
    CONFIG_WRITE_ACK_E
} signal_ack_t;

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

void generate_config(int config_fd) {

    // There are 6 int registers in config_reg_offset
    lseek(config_fd, 0, SEEK_SET);
    int regs[6];
    memset(regs, 0, 6);
    write(config_fd, regs, sizeof(regs));
}

int read_config(int config_fd, config_reg_offset_t offset)
{
    lseek(config_fd, offset, SEEK_SET);
    int ret = 0;
    read(config_fd, &ret, sizeof(ret));
    return ret;
}

void send_signal(int sig_fd, signal_ack_t ack, void *data, size_t n)
{
    // COBS data
    uint8_t buf0[sizeof(int) + 2];
    oe_cobs_stuff((uint8_t*)&ack, sizeof(int), buf0);
    write(sig_fd, &buf0, sizeof(buf0));

    uint8_t buf1[n+2];
    oe_cobs_stuff(data, n, buf1);
    write(sig_fd, &buf0, sizeof(buf1));

    uint8_t buf2[sizeof(int) + 2];
    signal_ack_t ack_e = ack + 1;
    oe_cobs_stuff((uint8_t *)&ack_e, sizeof(int), buf2);
    write(sig_fd, &buf0, sizeof(buf2));
}

int main()
{
    // Constants
    const size_t samp_per_block = num_chan * samp_per_chan_per_block;
    const size_t num_blocks = (run_time_sec * fs_hz) / samp_per_chan_per_block;
    const size_t sample_time_us = (1e6 * samp_per_chan_per_block ) / fs_hz;

    // FIFO file path
    const char *config_path = "/tmp/rat128_config";
    const char *sig_path = "/tmp/rat128_signal";
    const char *data_path = "/tmp/rat128_data";

    // Start fresh
    unlink(sig_path);
    unlink(data_path);
    unlink(config_path);

    // Async streams are similar to pipes
    mkfifo(data_path, 0666);
    mkfifo(sig_path, 0666);

    // Open FIFOs for write only and config file for read/write 
    // NB: Must respect this ordering when opening files in host or the two
    // programs will deadlock. The ordering is hidden in oe_init_ctx(), so its
    // up to firmware to do it correctly. I need to make sure this won't be an
    // issue when using xillybus
    int config_fd = open(config_path, O_RDWR | O_CREAT, 0666);
    int data_fd = open(data_path, O_WRONLY);
    int sig_fd = open(sig_path, O_WRONLY);

    // Generate config file content
    generate_config(config_fd);

    // Set data pipe capacity
    // Sample number, LFP data, ...
    fcntl(data_fd, F_SETPIPE_SZ, sizeof(uint64_t) + samp_per_block * sizeof(int16_t));

    // Populate config file

    // TODO: Start signal thread

    // Start data generation loop
    int i;
    uint64_t sample = 0;

    for (i = 0; i < num_blocks; i++) {

        // Generate data block
        // 1. Sample number
        write(data_fd, &sample, 8);

        // 2. LFP data
        int16_t lfp_block[samp_per_block];
        int j;
        for (j = 0; j < samp_per_block; j++) {
            lfp_block[j] = (int16_t)randn(0, 500);
            //int16_t tmp = (int16_t)randn(0, 500);
            //size_t rc = write(data_fd, &tmp, sizeof(int16_t));
        }

        size_t rc = write(data_fd, lfp_block, samp_per_block * sizeof(int16_t));
        printf("Write %zu bytes\n", rc);

        // Increment sample count
        sample += samp_per_chan_per_block;

        // Check for config reg states
        if (read_config(config_fd, CONFIG_READ_TRIG) > 0) {

            // ... Does config read stuff

            // Successful read, push result to signal stream
            int result = 123;
            //send_signal(sig_fd, CONFIG_READ_ACK, &result, sizeof(result));

        } else if (read_config(config_fd, CONFIG_WRITE_TRIG) > 0) {

            // ... Does config write stuff

            // Successful write, push result to signal stream
            int result = 123;
            //send_signal(sig_fd, CONFIG_WRITE_ACK, &result, sizeof(result));

        }
    }

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
