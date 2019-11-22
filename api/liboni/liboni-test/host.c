#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "oni.h"
#include "onidevices.h"
#include "oelogo.h"
#include "drivers\oepcie\liboni-driver-oepcie.h"

// Dump raw device streams to files?
//#define DUMPFILES

#ifdef DUMPFILES
FILE **dump_files;
#endif

// Windows- and UNIX-specific includes etc
#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "liboni")
#include <stdio.h>
#include <stdlib.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

volatile oni_ctx ctx = NULL;
oni_device_t *devices = NULL;
volatile int quit = 0;
volatile int display = 0;
volatile int display_clock = 0;
const int quiet = 1;
int running = 1;

int parse_reg_cmd(const char *cmd, long *values, int len)
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

    if (k < len)
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
    unsigned long this_cnt = 0;

    while (!quit)  {

        int rc = 0;
        oni_frame_t *frame;
        rc = oni_read_frame(ctx, &frame);
        if (rc < 0) {
            printf("Error: %s\n", oni_error_str(rc));
            quit = 1;
            break;
        }

        if (display_clock && counter % 1000 == 0)
            printf("\tSample: %" PRIu64 "\n\n", frame->clock);

        int i;
        for (i = 0; i < frame->num_dev; i++) {

            // Pull data
            if (i >= frame->num_dev)
                continue;

            size_t this_idx = frame->dev_idxs[i];

            oni_device_t this_dev = devices[this_idx];
            uint8_t *data = (uint8_t *)(frame->data + frame->dev_offs[i]);
            size_t data_sz = this_dev.read_size;

#ifdef DUMPFILES
            fwrite(data, 1, data_sz, dump_files[this_idx]);
#endif
            if (display && counter % 1000 == 0) { //this_idx == 19 && this_cnt < 500) { // && counter % 1000 == 0) { // && this_idx == 4 ) { //
                
                this_cnt++;
                printf("\tDev: %zu (%s) \n",
                    this_idx,
                    oni_device_str(this_dev.id));
                    // this_cnt);

                int i;
                printf("\tData: [");
                for (i = 0; i < data_sz; i += 2)
                    printf("%u ", *(uint16_t *)(data + i));
                printf("]\n");
            }
        }

        counter++;
        oni_destroy_frame(frame);
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
//            rc = oni_write(ctx, 0, data, sizeof(data));
//            printf("%d: %d\n", row_num, rc);
//
//            if (rc < 0) {
//                printf("Error: %s\n", oni_error_str(rc));
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
    // Default paths
    const char *config_path = ONI_DEFAULTCONFIGPATH;
    const char *sig_path = ONI_DEFAULTSIGNALPATH;
    const char *read_path = ONI_DEFAULTREADPATH;
    const char *write_path = ONI_DEFAULTWRITEPATH;

    printf(logo_med);
    if (argc != 1 && argc != 5) {
        printf("usage:\n");
        printf("\thost: run using default stream paths\n");
        printf("\thost config signal read write: specify the configuration, signal, read, and write paths.\n");
        exit(1);
    }
    else if (argc == 5) {

        // Set firmware paths
        config_path = argv[1];
        sig_path = argv[2];
        read_path = argv[3];
        write_path = argv[4];
    }
    
    // Return code
    int rc = ONI_ESUCCESS;

    // Generate context
    ctx = oni_create_ctx(OEPCIE_NAME);
    if (!ctx) exit(EXIT_FAILURE);

    // Set paths in context
    rc = oni_set_driver_opt(ctx, ONI_OEPCIE_CONFIGSTREAMPATH, config_path, strlen(config_path) + 1);
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
    rc = oni_set_driver_opt(ctx, ONI_OEPCIE_SIGNALSTREAMPATH, sig_path, strlen(sig_path) + 1);
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
    rc = oni_set_driver_opt(ctx, ONI_OEPCIE_READSTREAMPATH, read_path, strlen(read_path) + 1);
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
    rc = oni_set_driver_opt(ctx, ONI_OEPCIE_WRITESTREAMPATH, write_path, strlen(write_path) + 1);
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }

    // Set acqusition to run immediately following reset
    oni_reg_val_t immediate_run = 1;
    rc = oni_set_opt(ctx, ONI_RUNONRESET, &immediate_run, sizeof(oni_reg_val_t));
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }

    // Initialize context and discover hardware
    int rc = oni_init_ctx(ctx,-1);
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
    assert(rc == 0);

    // Examine device map
    oni_size_t num_devs = 0;
    size_t num_devs_sz = sizeof(num_devs);
    oni_get_opt(ctx, ONI_NUMDEVICES, &num_devs, &num_devs_sz);

    // Get the device map
    size_t devices_sz = sizeof(oni_device_t) * num_devs;
    devices = (oni_device_t *)realloc(devices, devices_sz);
    if (devices == NULL) { exit(EXIT_FAILURE); }
    oni_get_opt(ctx, ONI_DEVICEMAP, devices, &devices_sz);

#ifdef DUMPFILES
    // Make room for dump files
    dump_files = malloc(num_devs * sizeof(FILE *));
#endif

    // Show device map
    //printf("Found the following devices:\n");
    size_t dev_idx;
    for (dev_idx = 0; dev_idx < num_devs; dev_idx++) {

        const char *dev_str = oni_device_str(devices[dev_idx].id);

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

    oni_size_t frame_size = 0;
    size_t frame_size_sz = sizeof(frame_size);
    oni_get_opt(ctx, ONI_MAXREADFRAMESIZE, &frame_size, &frame_size_sz);
    printf("Max. read frame size: %u bytes\n", frame_size);

    oni_size_t block_size = 2048;
    size_t block_size_sz = sizeof(block_size);
    oni_set_opt(ctx, ONI_BLOCKREADSIZE, &block_size, block_size_sz);
    printf("Setting block read size to: %u bytes\n", block_size);

    oni_get_opt(ctx, ONI_BLOCKREADSIZE, &block_size, &block_size_sz);
    printf("Block read size: %u bytes\n", block_size);

    // Try to write to base clock freq, which is write only
    oni_size_t base_hz = (oni_size_t)10e6;
    rc = oni_set_opt(ctx, ONI_SYSCLKHZ, &base_hz, sizeof(oni_size_t));
    assert(rc == ONI_EREADONLY && "Successful write to read-only register.");

    size_t clk_val_sz = sizeof(base_hz);
    rc = oni_get_opt(ctx, ONI_SYSCLKHZ, &base_hz, &clk_val_sz);
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
    assert(!rc && "Register read failure.");
    printf("System clock rate: %u Hz\n", base_hz);

    // Start acquisition
    oni_size_t run = 1;
    rc = oni_set_opt(ctx, ONI_RUNNING, &run, sizeof(run));
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }

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
        printf("\tc - toggle 1/1000 clock display\n");
        printf("\td - toggle 1/1000 display\n");
        printf("\tp - toggle stream pause\n");
        printf("\tx - issue a hardware reset\n");
        printf("\tw - write to device register\n");
        printf("\tr - read from device register\n");
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
            oni_size_t run = running;
            rc = oni_set_opt(ctx, ONI_RUNNING, &run, sizeof(run));
            if (rc) {
                printf("Error: %s\n", oni_error_str(rc));
            }
            printf("Paused\n");
        }
        else if (c == 'x') {
            oni_size_t reset = 1;
            rc = oni_set_opt(ctx, ONI_RESET, &reset, sizeof(reset));
            if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
        }
        else if (c == 'c') {
            display_clock = (display_clock == 0) ? 1 : 0;
        }
        else if (c == 'd') {
            display = (display == 0) ? 1 : 0;
        }
        else if (c == 'w') {
            printf("Write to a device register.\n");
            printf("Enter: dev_idx reg_addr reg_val\n");
            printf(">>> ");

            // Read the command
            char *buf = NULL;
            size_t len = 0;
            rc = getline(&buf, &len, stdin);
            if (rc == -1) { printf("Error: bad command\n"); continue; }

            // Parse the command string
            long values[3];
            rc = parse_reg_cmd(buf, values, 3);
            if (rc == -1) { printf("Error: bad command\n"); continue; }
            free(buf);

            size_t dev_idx = (size_t)values[0];
            oni_size_t addr = (oni_size_t)values[1];
            oni_size_t val = (oni_size_t)values[2];

            oni_write_reg(ctx, dev_idx, addr, val);
        }
        else if (c == 'r') {
            printf("Read a device register.\n");
            printf("Enter: dev_idx reg_addr\n");
            printf(">>> ");

            // Read the command
            char *buf = NULL;
            size_t len = 0;
            rc = getline(&buf, &len, stdin);
            if (rc == -1) { printf("Error: bad command\n"); continue; }

            // Parse the command string
            long values[2];
            rc = parse_reg_cmd(buf, values, 2);
            if (rc == -1) { printf("Error: bad command\n"); continue; }
            free(buf);

            size_t dev_idx = (size_t)values[0];
            oni_size_t addr = (oni_size_t)values[1];

            oni_reg_val_t val = 0;
            oni_read_reg(ctx, dev_idx, addr, &val);

            printf("Reg. value: %u\n", val);
        }
    }

    // Join data and signal threads
    quit = 1;
#ifdef _WIN32

    WaitForSingleObject(read_thread, 200); // INFINITE);
    CloseHandle(read_thread);


    //WaitForSingleObject(write_thread, INFINITE);
    //CloseHandle(write_thread);
#else
    if(running)
        pthread_join(read_tid, NULL);
    //pthread_join(write_tid, NULL);
#endif
#ifdef DUMPFILES
    // Close dump files
    for (dev_idx = 0; dev_idx < num_devs; dev_idx++) {
        fclose(dump_files[dev_idx]);
    }
#endif

    // Free dynamic stuff
    oni_destroy_ctx(ctx);
    free(devices);

    return 0;
}
