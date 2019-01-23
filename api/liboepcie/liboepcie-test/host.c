#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "oepcie.h"
#include "oedevices.h"

// Dump raw device streams to files?
//#define DUMPFILES

#ifdef DUMPFILES
FILE **dump_files;
#endif

// Windows- and UNIX-specific includes etc
#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "liboepcie")
#include <stdio.h>
#include <stdlib.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

volatile oe_ctx ctx = NULL;
oe_device_t *devices = NULL;
volatile int quit = 0;
volatile int display = 0;
volatile int display_clock = 0;
int running = 1;

int parse_reg_cmd(const char *cmd, long *values)
{
    char *end;
    int k = 0;
    for (long i = strtol(cmd, &end, 10);
         cmd != end;
         i = strtol(cmd, &end, 10))
    {

        cmd = end;
        if (errno == ERANGE){ return -1; }

        values[k++] = i;
        if (k == 3)
            break;
    }

    if (k < 3)
        return -1;

    return 0;
}

#ifdef _WIN32
DWORD WINAPI read_loop(LPVOID lpParam)
#else
void *read_loop(void *vargp)
#endif
{
    unsigned long counter = 0;

    while (!quit)  {

        int rc = 0;
        oe_frame_t *frame;
        rc = oe_read_frame(ctx, &frame);
        if (rc < 0) {
            printf("Error: %s\n", oe_error_str(rc));
            quit = 1;
            break;
        }

        if (display_clock && counter % 100 == 0)
            printf("\tSample: %" PRIu64 "\n\n", frame->clock);

        int i;
        for (i = 0; i < frame->num_dev; i++) {

            // Pull data
            size_t this_idx = frame->dev_idxs[i];
            oe_device_t this_dev = devices[this_idx];
            uint8_t *data = (uint8_t *)(frame->data + frame->dev_offs[i]);
            size_t data_sz = this_dev.read_size;

#ifdef DUMPFILES
            fwrite(data, 1, data_sz, dump_files[this_idx]);
#endif
            if (display && counter % 100 == 0) {
                printf("\tDev: %zu (%s)\n",
                    this_idx,
                    oe_device_str(this_dev.id));

                int i;
                printf("\tData: [");
                for (i = 0; i < data_sz; i += 2)
                    printf("%u ", *(uint16_t *)(data + i));
                printf("]\n");
            }
        }

        counter++;
        oe_destroy_frame(frame);
    }

    return NULL;
}

//#ifdef _WIN32
//DWORD WINAPI write_loop(LPVOID lpParam)
//#else
//void *write_loop(void *vargp)
//#endif
//{
//    // Write zeros to device 3
//
//    while (!quit)  {
//
//        int row_num = 0;
//        while (row_num < 256) {
//
//            int rc = 0;
//            uint16_t data[275];
//
//            // Header
//            data[0] = 16384 + row_num;
//
//            // Row update
//            int i = 0;
//            for (i = 0; i < 256; i++)
//                data[i + 19]  = row_num;
//
//            rc = oe_write(ctx, 0, data, sizeof(data));
//            printf("%d: %d\n", row_num, rc);
//
//            if (rc < 0) {
//                printf("Error: %s\n", oe_error_str(rc));
//                quit = 1;
//                break;
//            }
//
//            row_num++;
//        }
//    }
//
//    return NULL;
//}

int main(int argc, char *argv[])
{
    // Generate context
    ctx = oe_create_ctx();
    if (!ctx) exit(EXIT_FAILURE);

    const char *config_path = OE_DEFAULTCONFIGPATH;
    const char *sig_path = OE_DEFAULTSIGNALPATH;
    const char *read_path = OE_DEFAULTREADPATH;
    const char *write_path = OE_DEFAULTWRITEPATH;

    if (argc != 1 && argc != 5) {
        printf("usage:\n");
        printf("\thost: run using default stream paths\n");
        printf("\thost config signal read write: specify the configuration, signal, read, and write paths.\n");
        exit(1);
    }

    if (argc == 5) {

        // Set firmware paths
        config_path = argv[1];
        sig_path = argv[2];
        read_path = argv[3];
        write_path = argv[4];
    }

    // Set paths in context
    oe_set_opt(ctx, OE_CONFIGSTREAMPATH, config_path, strlen(config_path) + 1);
    oe_set_opt(ctx, OE_SIGNALSTREAMPATH, sig_path, strlen(sig_path) + 1);
    oe_set_opt(ctx, OE_READSTREAMPATH, read_path, strlen(read_path) + 1);
    oe_set_opt(ctx, OE_WRITESTREAMPATH, write_path, strlen(write_path) + 1);

    // Initialize context and discover hardware
    int rc = oe_init_ctx(ctx);
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(rc == 0);

    // Examine device map
    oe_size_t num_devs = 0;
    size_t num_devs_sz = sizeof(num_devs);
    oe_get_opt(ctx, OE_NUMDEVICES, &num_devs, &num_devs_sz);

    // Get the device map
    size_t devices_sz = sizeof(oe_device_t) * num_devs;
    devices = (oe_device_t *)realloc(devices, devices_sz);
    if (devices == NULL) { exit(EXIT_FAILURE); }
    oe_get_opt(ctx, OE_DEVICEMAP, devices, &devices_sz);

#ifdef DUMPFILES
    // Make room for dump files
    dump_files = malloc(num_devs * sizeof(FILE *));
#endif

    // Show device map
    printf("Found the following devices:\n");
    size_t dev_idx;
    for (dev_idx = 0; dev_idx < num_devs; dev_idx++) {

        const char *dev_str = oe_device_str(devices[dev_idx].id);

        printf("\t%zd) ID: %d (%s), Read size: %u\n",
            dev_idx,
            devices[dev_idx].id,
            dev_str,
            devices[dev_idx].read_size);

#ifdef DUMPFILES
        // Open dump files
        char * buffer = malloc(100);
        snprintf(buffer, 100, "%s_idx-%zd_id-%d.raw", "dev", dev_idx, devices[dev_idx].id);
        dump_files[dev_idx] = fopen(buffer, "wb");
        free(buffer);
#endif
    }

    // Reset the hardware
    oe_size_t reset = 1;
    rc = oe_set_opt(ctx, OE_RESET, &reset, sizeof(reset));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register write failure.");

    // HACK: "wait" for reset acknowledgement. In real firmware, this will be actual async ACK.
    usleep(100e3);

    oe_size_t frame_size = 0;
    size_t frame_size_sz = sizeof(frame_size);
    oe_get_opt(ctx, OE_MAXREADFRAMESIZE, &frame_size, &frame_size_sz);
    printf("Max. read frame size: %u bytes\n", frame_size);

    size_t block_size = 2048;
    size_t block_size_sz = sizeof(block_size);
    oe_set_opt(ctx, OE_BLOCKREADSIZE, &block_size, block_size_sz);
    printf("Setting block read size to : %zu bytes\n", block_size);

    oe_get_opt(ctx, OE_BLOCKREADSIZE, &block_size, &block_size_sz);
    printf("Block read size: %zu bytes\n", block_size);

    // Try to write to base clock freq, which is write only
    oe_size_t base_hz = (oe_size_t)10e6;
    rc = oe_set_opt(ctx, OE_SYSCLKHZ, &base_hz, sizeof(oe_size_t));
    assert(rc == OE_EREADONLY && "Successful write to read-only register.");

    size_t clk_val_sz = sizeof(base_hz);
    rc = oe_get_opt(ctx, OE_SYSCLKHZ, &base_hz, &clk_val_sz);
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register read failure.");
    printf("System clock rate: %u Hz\n", base_hz);

    oe_size_t acq_hz = 0;
    size_t acq_clk_val_sz = sizeof(acq_hz);
    rc = oe_get_opt(ctx, OE_ACQCLKHZ, &acq_hz, &acq_clk_val_sz);
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register read failure.");
    printf("Acquisition clock rate: %u Hz\n", acq_hz);

    // Start acquisition
    oe_size_t run = 1;
    rc = oe_set_opt(ctx, OE_RUNNING, &run, sizeof(run));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }

    // Generate data read_thread and continue here config/signal handling in parallel
#ifdef _WIN32
    DWORD read_tid;
    HANDLE read_thread;
    read_thread = CreateThread(NULL, 0, read_loop, NULL, 0, &read_tid);

    //DWORD write_tid;
    //HANDLE write_thread;
    //write_thread = CreateThread(NULL, 0, write_loop, NULL, 0, &write_tid);
#else
    pthread_t read_tid;
    pthread_create(&read_tid, NULL, read_loop, NULL);
    //pthread_t write_tid;
    //pthread_create(&write_tid, NULL, write_loop, NULL);
#endif

    // Read stdin to start (s) or pause (p)
    int c = 's';
    while (c != 'q') {

        printf("Enter a command and press enter:\n");
        printf("\tc - toggle 1/100 clock display\n");
        printf("\td - toggle 1/100 display\n");
        printf("\tp - toggle stream pause\n");
        printf("\tr - enter register command\n");
        printf("\tq - quit\n");
        printf(">>> ");

        char *cmd = NULL;
        size_t cmd_len = 0;
        rc = getline(&cmd, &cmd_len, stdin);
        if (rc == -1) { printf("Error: bad command\n"); continue; }
        c = cmd[0];
        free(cmd);

        if (c == 'p') {
            running = (running == 1) ? 0 : 1;
            oe_size_t run = running;
            rc = oe_set_opt(ctx, OE_RUNNING, &run, sizeof(run));
            if (rc) {
                printf("Error: %s\n", oe_error_str(rc));
            }
            printf("Paused\n");
        }
        else if (c == 'c') {
            display_clock = (display_clock == 0) ? 1 : 0;
        }
        else if (c == 'd') {
            display = (display == 0) ? 1 : 0;
        }
        else if (c == 'r') {
            printf("Enter dev_idx reg_addr reg_val\n");
            printf(">>> ");

            // Read the command
            char *buf = NULL;
            size_t len = 0;
            rc = getline(&buf, &len, stdin);
            if (rc == -1) { printf("Error: bad command\n"); continue; }

            // Parse the command string
            long values[3];
            rc = parse_reg_cmd(buf, values);
            if (rc == -1) { printf("Error: bad command\n"); continue; }
            free(buf);

            size_t dev_idx = (size_t)values[0];
            oe_size_t addr = (oe_size_t)values[1];
            oe_size_t val = (oe_size_t)values[2];

            oe_write_reg(ctx, dev_idx, addr, val);
        }
    }

    // Join data and signal threads
    quit = 1;
#ifdef _WIN32
    WaitForSingleObject(read_thread, INFINITE);
    CloseHandle(read_thread);

    //WaitForSingleObject(write_thread, INFINITE);
    //CloseHandle(write_thread);
#else
    pthread_join(read_tid, NULL);
    //pthread_join(write_tid, NULL);
#endif
#ifdef DUMPFILES
    // Close dump files
    for (dev_idx = 0; dev_idx < num_devs; dev_idx++) {
        fclose(dump_files[dev_idx]);
    }
#endif

    // Reset the hardware
    reset = 1;
    rc = oe_set_opt(ctx, OE_RESET, &reset, sizeof(reset));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register write failure.");

    // Free dynamic stuff
    oe_destroy_ctx(ctx);
    free(devices);

    return 0;
}
