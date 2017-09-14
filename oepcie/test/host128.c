#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../liboepcie/oedevice.h"
#include "../liboepcie/oepcie.h"

// TODO: This should be retrieved from header packet 
// Params
const size_t num_chan  = 128;
const size_t samp_per_chan_per_block = 1000;
const size_t fs_hz = 30e3;

int main()
{
    // Generate context
    oe_ctx ctx = NULL;
    ctx = oe_create_ctx();

    // Set stream paths
    const char *config_path = "/tmp/rat128_config";
    const char *sig_path = "/tmp/rat128_signal";
    const char *data_path = "/tmp/rat128_data";

    // Async streams are similar to pipes
    // NB: Only needed if host starts before firmware
    mkfifo(sig_path, 0666);
    mkfifo(data_path, 0666);

    oe_set_ctx_opt(ctx, OE_CONFIGSTREAMPATH, config_path, strlen(config_path) + 1);
    oe_set_ctx_opt(ctx, OE_SIGNALSTREAMPATH, sig_path, strlen(sig_path) + 1);
    oe_set_ctx_opt(ctx, OE_DATASTREAMPATH, data_path, strlen(data_path) + 1);

    // TODO: Build from header packet constants
    //oe_write_config();    
    const size_t block_bytes = num_chan * samp_per_chan_per_block * 2;

    // Initialize context and discover hardware
    int rc = oe_init_ctx(ctx);
    printf("Return %d\n", rc);

    //assert(oe_init_ctx(ctx) == 0);

    // Examine device map
    oe_size_t num_devs = 0;
    size_t num_devs_sz = sizeof(num_devs);
    oe_get_ctx_opt(ctx, OE_NUMDEVICES, &num_devs, &num_devs_sz); 

    oe_device_t devices[num_devs];
    size_t devices_sz = sizeof(devices);
    oe_get_ctx_opt(ctx, OE_DEVICEMAP, devices, &devices_sz);

    printf("Found the following devices:\n");
    int dev_idx;
    for (dev_idx = 0; dev_idx < num_devs; dev_idx++) {

        printf("\t%d) ID: %d, Offset: %zu, Read size:%zu\n",
               dev_idx,
               devices[dev_idx].id,
               devices[dev_idx].read_offset,
               devices[dev_idx].read_size);
    }

    // Start acquisition
    oe_write_reg(ctx, OEPCIEMASTER, OEPCIEMASTER_RUNNING, 1);

    char buffer[8 + block_bytes];
    while (oe_read(ctx, &buffer, 8 + block_bytes)  >= 0)  {

        // Get sample number and print
        uint64_t sample = *(uint64_t *)buffer;
        printf("Sample: %" PRIu64 "\n", sample);

        // LFP data
        int16_t *lfp = (int16_t *)(buffer + 8);
        int i;
        printf("LFP: [");
        for (i =0; i < 10; i++)
            printf("%" PRId16 " ", *(lfp + i));
        printf("...]\n");

    }

    oe_destroy_ctx(ctx);

    return 0;
}
