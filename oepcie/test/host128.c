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
    oe_write_config();    
    const size_t block_bytes = num_chan * samp_per_chan_per_block * 2;

    assert(oe_init_ctx(ctx) == 0);

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
