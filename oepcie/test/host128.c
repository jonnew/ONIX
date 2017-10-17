#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../liboepcie/oepcie.h"

int main()
{
    // Frames per block read
    const size_t frames_per_read = 100;

    // Generate context
    oe_ctx ctx = NULL;
    ctx = oe_create_ctx();
    if (!ctx) exit(EXIT_FAILURE);

    // Set stream paths
    const char *config_path = "/tmp/rat128_config";
    const char *sig_path = "/tmp/rat128_signal";
    const char *data_path = "/tmp/rat128_data";

    oe_set_opt(ctx, OE_CONFIGSTREAMPATH, config_path, strlen(config_path) + 1);
    oe_set_opt(ctx, OE_SIGNALSTREAMPATH, sig_path, strlen(sig_path) + 1);
    oe_set_opt(ctx, OE_READSTREAMPATH, data_path, strlen(data_path) + 1);

    // Initialize context and discover hardware
    assert(oe_init_ctx(ctx) == 0);

    // Examine device map
    oe_size_t num_devs = 0;
    size_t num_devs_sz = sizeof(num_devs);
    oe_get_opt(ctx, OE_NUMDEVICES, &num_devs, &num_devs_sz);

    oe_device_t devices[num_devs];
    size_t devices_sz = sizeof(devices);
    oe_get_opt(ctx, OE_DEVICEMAP, devices, &devices_sz);

    printf("Found the following devices:\n");
    int dev_idx;
    for (dev_idx = 0; dev_idx < num_devs; dev_idx++) {

        const char *dev_str = oe_device_str(devices[dev_idx].id);

        printf("\t%d) ID: %d (%s), Offset: %u, Read size:%u\n",
               dev_idx,
               devices[dev_idx].id,
               dev_str,
               devices[dev_idx].read_offset,
               devices[dev_idx].read_size);
    }

    oe_size_t frame_size = 0;
    size_t frame_size_sz = sizeof(frame_size);
    oe_get_opt(ctx, OE_READFRAMESIZE, &frame_size, &frame_size_sz);
    printf("Frame size: %u bytes\n", frame_size);

    // Try to write to base clock freq, which is write only
    oe_reg_val_t base_hz = 10e6;
    int rc = oe_set_opt(ctx, OE_SYSCLKHZ, &base_hz, sizeof(oe_reg_val_t));
    assert(rc == OE_EREADONLY && "Succesful write to read-only register.");

    size_t clk_val_sz = sizeof(clk_val_sz);
    rc = oe_get_opt(ctx, OE_SYSCLKHZ, &base_hz, &clk_val_sz);
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register read failure.");

    //  Calculate m and d to get 10kHz
    uint32_t fs_desired = 20000;
    oe_reg_val_t m = 1;
    oe_reg_val_t d = (m * base_hz) / fs_desired;

    // Set clock divider to 10/100 to get 10kHz sample clock
    rc = oe_set_opt(ctx, OE_FSCLKM, &m, sizeof(m));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register write failure.");

    rc = oe_set_opt(ctx, OE_FSCLKD, &d, sizeof(d));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register write failure.");

    // HACK. Wait for fs update
    usleep(3000);

    uint32_t fs_hz;
    size_t fs_hz_sz = sizeof(fs_hz);
    rc = oe_get_opt(ctx, OE_FSCLKHZ, &fs_hz, &fs_hz_sz);
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register read failure.");
    assert(fs_hz == fs_desired && "Sample rate set failed.");

    // Start acquisition
    oe_reg_val_t run = 1;
    rc = oe_set_opt(ctx, OE_RUNNING, &run, sizeof(run));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }

    const size_t read_size = frame_size * frames_per_read;
    uint8_t buffer[frame_size * frames_per_read];

    while (oe_read(ctx, &buffer, read_size)  >= 0)  {

        printf("-- start read block --\n");

        int f;
        for (f = 0; f < frames_per_read; f++) {

            size_t frame_offset = f * frame_size;
            uint64_t sample = *(uint64_t *)(buffer + frame_offset);

            printf("Sample: %" PRIu64 "\n", sample);

            for (dev_idx = 0; dev_idx < num_devs; dev_idx++) {

                oe_device_t this_dev = devices[dev_idx];

                printf("\tDev: %d (%s)\n", dev_idx, oe_device_str(this_dev.id));
                int16_t *lfp = (int16_t *)((uint8_t *)buffer + this_dev.read_offset);
                int i;
                printf("\tData: [");
                for (i = 0; i < 10; i++)
                    printf("%" PRId16 " ", *(lfp + i));
                printf("...]\n");
            }
        }
    }

    // Reset the hardware
    oe_reg_val_t reset = 1;
    rc = oe_set_opt(ctx, OE_RESET, &reset, sizeof(reset));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }

    assert(!rc && "Register write failure.");

    oe_destroy_ctx(ctx);

    return 0;
}
