#define _GNU_SOURCE // fcntl

#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "oni.h"
#include "onidevices.h"

#include "testfunc.h"

// Number of samples generate
int num_samp = -1;
timespec_t start_time, end_time;

// Data acq. params
const size_t approx_frame_size = 500;
const size_t fifo_frame_capacity = 1000;

// Config registers
volatile oni_size_t running = 0;
const oni_size_t sys_clock_hz = 100e6;
const oni_size_t acq_clock_hz = 42e6;

// Global state
volatile uint64_t sample_tick = 0;

// Thread control
volatile int quit = 0;

// FIFO file path and desciptor handles to mimic xillybus streams
const char *config_path = "/tmp/xillybus_cmd_32";
const char *sig_path = "/tmp/xillybus_signal_8";
const char *read_path = "/tmp/xillybus_data_read_32";
const char *write_path = "/tmp/xillybus_data_write_32";

int config_fd = -1;
int read_fd = -1;
int write_fd = -1;
int sig_fd = -1;

// These enums are REDEFINED here (from oni.c) because they are static there.
typedef enum oni_signal {
    NULLSIG             = (1u << 0),
    CONFIGWACK          = (1u << 1), // Configuration write-acknowledgement
    CONFIGWNACK         = (1u << 2), // Configuration no-write-acknowledgement
    CONFIGRACK          = (1u << 3), // Configuration read-acknowledgement
    CONFIGRNACK         = (1u << 4), // Configuration no-read-acknowledgement
    DEVICEMAPACK        = (1u << 5), // Device map start acnknowledgement
    DEVICEINST          = (1u << 6), // Deivce map instance
} oni_signal_t;

// Configuration file offsets
typedef enum oni_conf_reg_off {

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
    CONFACQCLKHZOFFSET  = 32,  // Configuration acquisition clock frequency register byte offset
} oni_conf_reg_off_t;

// Devices handled by this firmware
static oni_device_t my_devices[]
    = {{.id = ONI_RHD2164,
        .read_size = 67 * sizeof(uint16_t),
        .num_reads = 1,
        .write_size = 0,
        .num_writes = 1},
       {.id = ONI_RHD2164,
        .read_size = 67 * sizeof(uint16_t),
        .num_reads = 1,
        .write_size = 0,
        .num_writes = 1},
       {.id = ONI_MPU9250,
        .read_size = 9 * sizeof(uint16_t),
        .num_reads = 1,
        .write_size = 0,
        .num_writes = 1},
       {.id = 10000, // Non-standard device
        .read_size = 550,
        .num_reads = 256,
        .write_size = 550,
        .num_writes = 256}};


uint32_t get_read_frame_size(oni_device_t *dev_map, int *devs, size_t n_devs)
{
    int i;
    size_t read_frame_size = ONI_RFRAMEHEADERSZ + sizeof(uint32_t) * n_devs;
    for (i = 0; i < n_devs; i++) {

        oni_device_t cur_dev = dev_map[devs[i]];
        read_frame_size += cur_dev.read_size;
    }

    read_frame_size += read_frame_size % 4; // padding

    return read_frame_size;
}

//uint32_t get_write_frame_size(oni_device_t *dev_map, int *devs, size_t n_devs)
//{
//    int i;
//    size_t write_frame_size = ONI_RFRAMEHEADERSZ + sizeof(uint32_t) * n_devs;
//    for (i = 0; i < n_devs; i++) {
//
//        oni_device_t cur_dev = dev_map[devs[i]];
//        write_frame_size += cur_dev.read_size;
//    }
//
//    write_frame_size += write_frame_size % 4; // padding
//
//    return write_frame_size;
//}

int read_config(int config_fd, oni_conf_reg_off_t offset, void *result, size_t size)
{
    lseek(config_fd, offset, SEEK_SET);
    return read(config_fd, result, size);
}

int write_config(int config_fd, oni_conf_reg_off_t offset, void *value, size_t size)
{
    lseek(config_fd, offset, SEEK_SET);
    return write(config_fd, value, size);
}

void generate_default_config(int config_fd)
{
    // Default config
    running = 0;
    sample_tick = 0;

    // Just put a bunch of 0s in there
    lseek(config_fd, 0, SEEK_SET);
    oni_size_t regs[100] = {0};
    write(config_fd, regs, 100 * sizeof(oni_size_t));

    // Write defaults for registers that need it
    write_config(config_fd, CONFRUNNINGOFFSET, (void *)&running, sizeof(oni_size_t));
    write_config(config_fd, CONFSYSCLKHZOFFSET, (void *)&sys_clock_hz, sizeof(oni_size_t));
    write_config(config_fd, CONFACQCLKHZOFFSET, (void *)&acq_clock_hz, sizeof(oni_size_t));
}

int send_msg_signal(int sig_fd, oni_signal_t type)
{
    // Src and dst buffers
    // COBS data, 1 overhead byte + 0x0 delimiter
    uint8_t dst[sizeof(oni_signal_t) + 2] = {0};

    oni_cobs_stuff(dst, (uint8_t *)&type, sizeof(type));
    return write(sig_fd, dst, sizeof(dst));
}

int send_data_signal(int sig_fd, oni_signal_t type, void *data, size_t n)
{
    size_t packet_size = sizeof(oni_signal_t) + n;

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
    oni_cobs_stuff(dst, src, packet_size);

    // COBS data, 1 overhead byte + 0x0 delimiter
    return write(sig_fd, dst, packet_size + 2);
}

void send_device_map(int sig_fd)
{
    uint32_t num_dev = sizeof(my_devices) / sizeof(oni_device_t);
    send_data_signal(sig_fd, DEVICEMAPACK, &num_dev, sizeof(num_dev));

    // Loop through devices
    int i;
    for (i = 0; i < num_dev; i++)
        send_data_signal(sig_fd, DEVICEINST, &my_devices[i], sizeof(oni_device_t));
}

size_t make_frame(uint8_t **frame) {

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
        *frame = malloc(frame_size);

        // Device indicies
        *(((uint32_t *)(*frame + ONI_RFRAMEHEADERSZ)) + 0) = 0;
        *(((uint32_t *)(*frame + ONI_RFRAMEHEADERSZ)) + 1) = 1;
        *(((uint32_t *)(*frame + ONI_RFRAMEHEADERSZ)) + 2) = 2;

    } else {
        num_devs = 2;
        int dev_idxs[] = {0, 1};
        int i;
        for (i = 0; i < num_devs; i++)
            data_block_size += my_devices[dev_idxs[i]].read_size;

        frame_size
            = get_read_frame_size(my_devices, dev_idxs, num_devs);
        *frame = malloc(frame_size);

        // Device indicies
        *(((uint32_t *)(*frame + ONI_RFRAMEHEADERSZ)) + 0) = 0;
        *(((uint32_t *)(*frame + ONI_RFRAMEHEADERSZ)) + 1) = 1;
    }

    // Frame header
    *(uint64_t *)*frame = sample_tick;                      // Sample number
    *((uint16_t *)(*frame + ONI_RFRAMENDEVOFF)) = num_devs;  // Num devices
    *(*frame + ONI_RFRAMENERROFF) = 0;                       // Error

    // Where does the data block start and end
    uint8_t *data_ptr
        = *frame + (ONI_RFRAMEHEADERSZ + num_devs * sizeof(uint32_t));
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

    return frame_size;
}

// Write from host to firmware
void *write_loop(void *vargp)
{
    while (!quit) {

        oni_size_t dev_idx;
        ssize_t rc = read(write_fd, &dev_idx, sizeof(oni_size_t));

        // Return without read due to quit
        if (quit) break;

        assert(rc == sizeof(oni_size_t) && "Incomplete read.");

        oni_device_t this_dev = my_devices[dev_idx];
        char *buffer = malloc(this_dev.write_size);
        rc = read(write_fd, buffer, this_dev.write_size);

        // Return without read due to quit
        if (quit) break;

        assert(rc == this_dev.write_size && "Incomplete read.");

        // Dump to terminal
        printf("Host wrote to device at index %u:\n[", dev_idx);
        int i;
        for (i = 0; i < this_dev.write_size; i++)
            printf("%02x ", buffer[i]);
        printf("]\n");
    }

    return NULL;
}

// Read from firmware to host
void *read_loop(void *vargp)
{
    // Set data pipe capacity
    // Sample number, LFP data, ...
    fcntl(read_fd, F_SETPIPE_SZ, approx_frame_size * fifo_frame_capacity);

    // Static raw frame used in performance testing is enabled
    uint8_t *static_frame;
    size_t static_frame_size = make_frame(&static_frame);

    while (!quit) {

        if (sample_tick == num_samp) {
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            quit = 1;
        }

        if (running) {

            if (num_samp <= 0) {

                // Generate raw frame
                uint8_t *frame;
                size_t frame_size = make_frame(&frame);

                size_t rc = write(read_fd, frame, frame_size);
                assert(rc == frame_size && "Incomplete write.");

                free(frame);

            } else { // Performance test

                size_t rc = write(read_fd, static_frame, static_frame_size);
                assert(rc == static_frame_size && "Incomplete write.");
            }

            // Increment frame count
            sample_tick += 1;

        } else {
            usleep(1000); // Prevent CPU tacking

            // Get new start time
            clock_gettime(CLOCK_MONOTONIC, &start_time);
        }
    }

    free(static_frame);

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 1 && argc != 2) {
usage:
        printf("usage:\n");
        printf("\tfirmware: generate fake dataa indefinately\n");
        printf("\tfirmware num_frames: create a single, static frame and push "
               "it num_frames times for performance testing.\n");
        exit(1);
    }

    if (argc == 2) {
        num_samp = atoi(argv[1]);

        if (num_samp <= 0)
            goto usage;
    }

    // Start fresh
    unlink(sig_path);
    unlink(read_path);
    unlink(write_path);
    unlink(config_path);

    // Async streams are similar to pipes
    mkfifo(sig_path, 0666);
    mkfifo(read_path, 0666);
    mkfifo(write_path, 0666);

    // Open FIFOs for write only and config file for read/write

    // NB: This must be done first to prevent race condition -- host will try
    // to read noexistant file if fifos are opened before this. This should not
    // be an issue with xillybus.
    config_fd = open(config_path, O_RDWR | O_CREAT, 0666);

    // NB: Must respect this ordering when opening files in host or the two
    // programs will deadlock. The ordering is hidden in oni_init_ctx(), so its
    // up to firmware to do it correctly. I need to make sure this won't be an
    // issue when using xillybus
    sig_fd = open(sig_path, O_WRONLY);
    read_fd = open(read_path, O_WRONLY);
    write_fd = open(write_path, O_RDONLY);

    // Generate data thread and continue here config/signal handling in parallel
    pthread_t tid_read;
    pthread_create(&tid_read, NULL, read_loop, NULL);

    pthread_t tid_write;
    if (num_samp == -1)
        pthread_create(&tid_write, NULL, write_loop, NULL);

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
        oni_size_t run;
        read_config(config_fd, CONFRUNNINGOFFSET, &run, 4);
        running = run;

        oni_size_t reset;
        read_config(config_fd, CONFRESETOFFSET, &reset, 4);
        if (reset) {
            reset = 0; // untrigger
            write_config(config_fd, CONFRESETOFFSET, &reset, 4);
            goto reset;
        }

        // -- Device configuration Interface --
        // Poll configuration registers
        // NB: It is very important to 0-initalize the register values here
        // because (I think) they are cast to void * in read_config and
        // this is bad for some reason
        oni_size_t dev_id;
        read_config(config_fd, CONFDEVIDOFFSET, &dev_id, 4);
        // printf("Dev ID: %d\n", dev_id);

        oni_size_t reg_addr;
        read_config(config_fd, CONFADDROFFSET, &reg_addr, 4);
        // printf("Reg. Addr: %u\n", reg_addr);

        oni_size_t reg_val;
        read_config(config_fd, CONFVALUEOFFSET, &reg_val, 4);
        // printf("Reg. val.: %u\n", reg_val);

        oni_size_t reg_rw;
        read_config(config_fd, CONFRWOFFSET, &reg_rw, 1);
        // printf("Reg. RW: %hhu\n", reg_rw);

        oni_size_t trig_value;
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
    pthread_join(tid_read, NULL);

    if (num_samp == -1)
        pthread_join(tid_write, NULL);

    // Report runtime if nessesary
    if (num_samp > 0) {
        timespec_t diff = timediff(start_time, end_time);
        printf("Produced %d samples in %ld seconds and %ld nsec.\n",
               num_samp,
               diff.tv_sec,
               diff.tv_nsec);
    }

    // Close pipes/files
    close(sig_fd);
    close(read_fd);
    close(write_fd);
    close(config_fd);

    // Delete files
    unlink(sig_path);
    unlink(read_path);
    unlink(write_path);
    unlink(config_path);

    return 0;
}
