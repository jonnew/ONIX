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
    // Generate context
    oe_ctx ctx = NULL;
    ctx = oe_create_ctx();
    if (!ctx) exit(EXIT_FAILURE);

    // Set stream paths
    const char *config_path = "/tmp/rat128_config";
    const char *sig_path = "/tmp/rat128_signal";
    const char *data_path = "/tmp/rat128_read";
    //const char *config_path = "/dev/xillybus_cmd_mem_32";
    //const char *sig_path = "/dev/xillybus_async_read_8";
    //const char *data_path = "/dev/xillybus_data_read_32";

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

        printf("\t%d) ID: %d (%s), Read size:%u\n",
               dev_idx,
               devices[dev_idx].id,
               dev_str,
               devices[dev_idx].read_size);
    }

    oe_size_t frame_size = 0;
    size_t frame_size_sz = sizeof(frame_size);
    oe_get_opt(ctx, OE_READFRAMESIZE, &frame_size, &frame_size_sz);
    printf("Frame size: %u bytes\n", frame_size);

    // Try to write to base clock freq, which is write only
    oe_reg_val_t base_hz = 10e6;
    int rc = oe_set_opt(ctx, OE_SYSCLKHZ, &base_hz, sizeof(oe_reg_val_t));
    assert(rc == OE_EREADONLY && "Successful write to read-only register.");

    size_t clk_val_sz = sizeof(base_hz);
    rc = oe_get_opt(ctx, OE_SYSCLKHZ, &base_hz, &clk_val_sz);
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }
    assert(!rc && "Register read failure.");

    // Start acquisition
    oe_reg_val_t run = 1;
    rc = oe_set_opt(ctx, OE_RUNNING, &run, sizeof(run));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }

    rc = 0;
    oe_frame_t *frame;
    while (rc == 0)  {
        printf("-- Read frame --\n");

        rc = oe_read_frame(ctx, &frame);
        printf("\tSample: %" PRIu64 "\n", frame->clock);

        int i;
        for (i =  0; i < frame->num_dev; i++) {

            oe_device_t this_dev = devices[frame->dev_idxs[i]];

            printf("\tDev: %d (%s)\n", frame->dev_idxs[i], oe_device_str(this_dev.id));

            //int16_t *lfp
            //    = (int16_t *)((uint8_t *)buffer + this_dev.read_offset);
            //int i;
            //printf("\tData: [");
            //for (i = 0; i < 10; i++)
            //    printf("%" PRId16 " ", *(lfp + i));
            //printf("...]\n");
        }
    }

    oe_destroy_frame(frame);

    // Reset the hardware
    oe_reg_val_t reset = 1;
    rc = oe_set_opt(ctx, OE_RESET, &reset, sizeof(reset));
    if (rc) { printf("Error: %s\n", oe_error_str(rc)); }

    assert(!rc && "Register write failure.");

    oe_destroy_ctx(ctx);

    return 0;
}
