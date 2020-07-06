#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#include "oni.h"
#include "onidevices.h"
#include "oelogo.h"

// Dump raw device streams to files?
#define DUMPFILES

// Turn on RT optimization
#define RT

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
oni_size_t num_devs = 0;
oni_device_t *devices = NULL;
volatile int quit = 0;
volatile int display = 0;
const int quiet = 1;
int running = 1;

#ifdef _WIN32
HANDLE read_thread;
HANDLE write_thread;
#else
pthread_t read_tid;
pthread_t write_tid;
#endif

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

// Simple & slow device lookup
int find_dev(oni_dev_idx_t idx)
{
    int i;
    for (i = 0; i < num_devs; i++)
        if (devices[i].idx == idx)
            return i;

    return -1;
}

int16_t last_sample = 32767;
uint32_t out_count = 0;

#ifdef _WIN32
DWORD WINAPI read_loop(LPVOID lpParam)
#else
void *read_loop(void *vargp)
#endif
{
    unsigned long counter = 0;
    unsigned long this_cnt = 0;

    //// Pre-allocate write frame
    //oni_frame_t *w_frame = NULL;
    //oni_create_frame(ctx, &w_frame, 8, 4);

    while (!quit)  {

        int rc = 0;
        oni_frame_t *frame = NULL;
        rc = oni_read_frame(ctx, &frame);
        if (rc < 0) {
            printf("Error: %s\n", oni_error_str(rc));
            quit = 1;
            break;
        }

        int i = find_dev(frame->dev_idx);
        if (i == -1) goto next;

#ifdef DUMPFILES
        fwrite(frame->data, 1, frame->data_sz, dump_files[i]);
#endif

        if (display
            && counter % 1000 == 0) {
            //&& devices[i].id == ONI_RHD2164) {

            oni_device_t this_dev = devices[i];

            this_cnt++;
            printf("\tDev: %zu (%s) \n",
                frame->dev_idx,
                oni_device_str(this_dev.id));
                // this_cnt);

            oni_fifo_dat_t i;
            printf("\tData: [");
            for (i = 0; i < frame->data_sz; i += 2)
                printf("%u ", *(uint16_t *)(frame->data + i));
            printf("]\n");
        }

         //// Feedback loop test
         //if (frame->dev_idx == 7) {
         //
         //    int16_t sample = *(int16_t *)(frame->data + 10);
         //
         //    if (sample - last_sample > 500) {
         //
         //        memcpy(w_frame->data, &out_count, 4);
         //        out_count++;

         //        int rc = oni_write_frame(ctx, w_frame);
         //        if (rc < 0) { printf("Error: %s\n", oni_error_str(rc)); }

         //    }
         //
         //    last_sample = sample;
         //}
next:
        counter++;
        oni_destroy_frame(frame);
    }

    //oni_destroy_frame(w_frame);

    return NULL;
}

#ifdef _WIN32
DWORD WINAPI write_loop(LPVOID lpParam)
#else
void *write_loop(void *vargp)
#endif
{
    // Pre-allocate write frame
    // TODO: hardcoded dev_idx not good
    oni_frame_t *w_frame = NULL;
    int rc = oni_create_frame(ctx, &w_frame, 7, 24);
    if (rc < 0) {
        printf("Error: %s\n", oni_error_str(rc));
        goto error;
    }

    // Loop count
    uint32_t count = 0;

    // Cycle through writable devices and write counter to their data
    while (!quit) {

        memcpy(w_frame->data, &out_count, 4);
        out_count++;

        int rc = oni_write_frame(ctx, w_frame);
        if (rc < 0) {
            printf("Error: %s\n", oni_error_str(rc));
            goto error;
        }

        count++;

        Sleep(1);
    }

error:
    oni_destroy_frame(w_frame);
    return NULL;
}

void start_threads()
{
    // Start acquisition
    quit = 0;
    oni_size_t run = 1;
    int rc = oni_set_opt(ctx, ONI_OPT_RUNNING, &run, sizeof(run));
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }

    // Generate data read_thread and continue here config/signal handling in parallel
#ifdef _WIN32
    DWORD read_tid;
    read_thread = CreateThread(NULL, 0, read_loop, NULL, 0, &read_tid);

    DWORD write_tid;
    write_thread = CreateThread(NULL, 0, write_loop, NULL, 0, &write_tid);

#ifdef RT
    if (!SetThreadPriority(read_thread, THREAD_PRIORITY_TIME_CRITICAL))
        printf("Unable to set read thread priority.\n");
    if (!SetThreadPriority(write_thread, THREAD_PRIORITY_HIGHEST))
        printf("Unable to set read thread priority.\n");
#endif

#else
    pthread_create(&read_tid, NULL, read_loop, NULL);
    pthread_create(&write_tid, NULL, write_loop, NULL);
#endif
}

void stop_threads()
{
    // Join data and signal threads
    quit = 1;

#ifdef _WIN32

    WaitForSingleObject(read_thread, 200); // INFINITE);
    CloseHandle(read_thread);

    WaitForSingleObject(write_thread, 200);
    CloseHandle(write_thread);
#else
    if (running)
        pthread_join(read_tid, NULL);
    //pthread_join(write_tid, NULL);
#endif

    oni_size_t run = 0;
    int rc = oni_set_opt(ctx, ONI_OPT_RUNNING, &run, sizeof(run));
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
}

void print_dev_table(oni_device_t *devices, int num_devs)
{
    // Show device table
    printf(
        "     "
        "+------------------+-------+-------+-------+-------+-------+-----\n");
    printf("     |        \t\t|  \t|Read\t|Reads/\t|Wrt.\t|Wrt./ \t|     \n");
    printf("     |Dev. idx\t\t|ID\t|size\t|frame \t|size\t|frame \t|Desc.\n");
    printf("+----+------------------+-------+-------+-------+-------+-------+-----\n");

    size_t dev_idx;
    for (dev_idx = 0; dev_idx < num_devs; dev_idx++) {

        const char *dev_str = oni_device_str(devices[dev_idx].id);

        printf("|%02zd  |%05zd: 0x%02x.0x%02x\t|%d\t|%u\t|%u\t|%u\t|%u\t|%s\n",
               dev_idx,
               devices[dev_idx].idx,
               (uint8_t)(devices[dev_idx].idx >> 8),
               (uint8_t)devices[dev_idx].idx,
               devices[dev_idx].id,
               devices[dev_idx].read_size,
               devices[dev_idx].num_reads,
               devices[dev_idx].write_size,
               devices[dev_idx].num_writes,
               dev_str);
    }

    printf("+----+-------------------+-------+-------+-------+-------+-------+-----\n");
}

int main(int argc, char *argv[])
{
    printf(oe_logo_med);

    int host_idx = 0;
    char *driver;

    if (argc != 2 && argc != 3) {
        printf("usage:\n");
        printf("\thost driver [host_index] ...\n");
        exit(1);
    }

    driver = argv[1];

    if (argc == 3)
        host_idx = atoi(argv[2]);

#if defined(_WIN32) && defined(RT)
    if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
        printf("Failed to set thread priority\n");
        exit(EXIT_FAILURE);
    }
#else
    // TODO
#endif

    // Return code
    int rc = ONI_ESUCCESS;

    // Generate context
    ctx = oni_create_ctx("riffa");
    if (!ctx) { printf("Failed to create context\n"); exit(EXIT_FAILURE); }

    // Set acquisition to run immediately following reset
    oni_reg_val_t immediate_run = 0;
    rc = oni_set_opt(ctx, ONI_OPT_RUNONRESET, &immediate_run, sizeof(oni_reg_val_t));
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }

    // Initialize context and discover hardware
    rc = oni_init_ctx(ctx, -1);
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
    assert(rc == 0);

    // Examine device table
    size_t num_devs_sz = sizeof(num_devs);
    oni_get_opt(ctx, ONI_OPT_NUMDEVICES, &num_devs, &num_devs_sz);

    // Get the device table
    size_t devices_sz = sizeof(oni_device_t) * num_devs;
    devices = (oni_device_t *)realloc(devices, devices_sz);
    if (devices == NULL) { exit(EXIT_FAILURE); }
    oni_get_opt(ctx, ONI_OPT_DEVICETABLE, devices, &devices_sz);

#ifdef DUMPFILES
    // Make room for dump files
    dump_files = malloc(num_devs * sizeof(FILE *));

    // Open dump files
    for (size_t i = 0; i < num_devs; i++) {
        char * buffer = malloc(100);
        snprintf(buffer, 100, "%s_idx-%zd_id-%d.raw", "dev", i, devices[i].id);
        dump_files[i] = fopen(buffer, "wb");
        free(buffer);
    }
#endif

    // Show device table
    print_dev_table(devices, num_devs);

    oni_size_t frame_size = 0;
    size_t frame_size_sz = sizeof(frame_size);
    oni_get_opt(ctx, ONI_OPT_MAXREADFRAMESIZE, &frame_size, &frame_size_sz);
    printf("Max. read frame size: %u bytes\n", frame_size);

    oni_size_t block_size = 1024;
    size_t block_size_sz = sizeof(block_size);
    oni_set_opt(ctx, ONI_OPT_BLOCKREADSIZE, &block_size, block_size_sz);
    printf("Setting block read size to: %u bytes\n", block_size);

    oni_get_opt(ctx, ONI_OPT_BLOCKREADSIZE, &block_size, &block_size_sz);
    printf("Block read size: %u bytes\n", block_size);

    block_size = 8192;
    block_size_sz = sizeof(block_size);
    oni_set_opt(ctx, ONI_OPT_BLOCKWRITESIZE, &block_size, block_size_sz);
    printf("Setting write pre-allocation buffer to: %u bytes\n", block_size);

    oni_get_opt(ctx, ONI_OPT_BLOCKWRITESIZE, &block_size, &block_size_sz);
    printf("Write pre-allocation size: %u bytes\n", block_size);

    // Try to write to base clock freq, which is write only
    oni_size_t base_hz = (oni_size_t)10e6;
    rc = oni_set_opt(ctx, ONI_OPT_SYSCLKHZ, &base_hz, sizeof(oni_size_t));
    assert(rc == ONI_EREADONLY && "Successful write to read-only register.");

    size_t clk_val_sz = sizeof(base_hz);
    rc = oni_get_opt(ctx, ONI_OPT_SYSCLKHZ, &base_hz, &clk_val_sz);
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
    assert(!rc && "Register read failure.");
    printf("System clock rate: %u Hz\n", base_hz);

    // Start acquisition
    start_threads();

    // Read stdin to start (s) or pause (p)
    printf("Some commands can cause hardware malfunction if issued in the wrong order!\n");
    int c = 'x';
    while (c != 'q') {

        printf("Enter a command and press enter:\n");
        printf("\tc - toggle 1/1000 clock display\n");
        printf("\td - toggle 1/1000 display\n");
        printf("\tt - print device table\n");
        printf("\tp - toggle pause register\n");
        printf("\ts - toggle pause register & r/w thread operation\n");
        printf("\tr - read from device register\n");
        printf("\tw - write to device register\n");
        printf("\tx - issue a hardware reset\n");
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
            rc = oni_set_opt(ctx, ONI_OPT_RUNNING, &run, sizeof(run));
            if (rc) {
                printf("Error: %s\n", oni_error_str(rc));
            }
            printf("Paused\n");
        }
        else if (c == 'x') {
            oni_size_t reset = 1;
            rc = oni_set_opt(ctx, ONI_OPT_RESET, &reset, sizeof(reset));
            if (rc) { printf("Error: %s\n", oni_error_str(rc)); }
        }
        else if (c == 'd') {
            display = (display == 0) ? 1 : 0;
        }
        else if (c == 't') {
            print_dev_table(devices, num_devs);
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

            rc = oni_write_reg(ctx, dev_idx, addr, val);
            printf("%s\n", oni_error_str(rc));
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
            rc = oni_read_reg(ctx, dev_idx, addr, &val);
            if (!rc) {
                printf("Reg. value: %u\n", val);
            } else {
                printf("%s\n", oni_error_str(rc));
            }
        }
        else if (c == 's') {

            if (quit == 0) {
                stop_threads();
            }
            else {
                start_threads();
            }
        }
    }

    stop_threads();

#ifdef DUMPFILES
    // Close dump files
    for (int dev_idx = 0; dev_idx < num_devs; dev_idx++) {
        fclose(dump_files[dev_idx]);
    }
#endif

    // Stop hardware
    oni_size_t run = 0 ;
    rc = oni_set_opt(ctx, ONI_OPT_RUNNING, &run, sizeof(run));
    if (rc) { printf("Error: %s\n", oni_error_str(rc)); }

    // Free dynamic stuff
    oni_destroy_ctx(ctx);
    free(devices);

    return 0;
}
