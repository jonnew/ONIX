#include "errno.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#include "oepcie.h"
#include "oedevice.h"

typedef struct stream_fid {
    char* path;
    int fid;
} stream_fid_t;

typedef struct dev_map {
    int num_dev;
    device_t* devs;
} dev_map_t;

typedef enum run_state {
    INITIALIZED,
    UNINITIALIZED
} run_state_t;

typedef struct oe_ctx_impl {
    stream_fid_t header;
    stream_fid_t config;
    stream_fid_t data;
    stream_fid_t signal;
    dev_map_t map;

    run_state_t init;
    int config_id;
} oe_ctx_impl_t;

oe_ctx oe_create_ctx()
{
    oe_ctx ctx = calloc(1, sizeof(struct oe_ctx_impl));

    ctx->header.path = calloc(0, sizeof(char));
    ctx->config.path = calloc(0, sizeof(char));
    ctx->data.path = calloc(0, sizeof(char));
    ctx->signal.path = calloc(0, sizeof(char));

    ctx->init = UNINITIALIZED;
    ctx->config_id = -1;

    return ctx;
}

int oe_init_ctx(oe_ctx ctx)
{
    if (ctx->init == INITIALIZED)
        return OE_EREINITCTX;

    // Open all filestreams
    ctx->header.fid = open(ctx->header.path, O_RDONLY);
    if (ctx->header.fid == -1)
        return OE_EPATHINVALID;

    ctx->config.fid = open(ctx->config.path, O_RDWR);
    if (ctx->config.fid == -1)
        return OE_EPATHINVALID;

    ctx->data.fid = open(ctx->data.path, O_RDONLY);
    if (ctx->data.fid == -1)
        return OE_EPATHINVALID;

    ctx->data.fid = open(ctx->signal.path, O_RDONLY);
    if (ctx->signal.fid == -1)
        return OE_EPATHINVALID;

    // Get general configuration magic number
    read(ctx->header.fid, &(ctx->config_id), sizeof(int));

    int device_id = 0;
    int num_dev = 0;

    // TODO: This for loop will not terminate if the header file is empty,
    // must have terminating -1... consider defining MAX_num_dev
    while (device_id != -1) {

        read(ctx->header.fid, &device_id, sizeof(int));

        if (device_id > MAX_DEVICE_ID || device_id < -1)
            return OE_EDEVID;
        if (device_id != -1)
            num_dev++;
    }

    ctx->map.num_dev = num_dev;
    ctx->map.devs = calloc(num_dev, sizeof(device_t));

    // return to start of device_ids
    lseek(ctx->header.fid, 4, SEEK_SET);

    int i;
    for (i = 0; i < num_dev; i++) {
        int id = -1;
        if (read(ctx->header.fid, &id, sizeof(int)) == -1)
            return OE_EREADFAILURE;
        ctx->map.devs[i] = devices[id];
    }

    // Calculate read offsets
    ctx->map.devs[0].read_offset = 0;
    for (i = 1; i < num_dev; i++) {
        ctx->map.devs[i].read_offset
            = ctx->map.devs[i - 1].read_offset + ctx->map.devs[i - 1].read_size;
    }

    // TODO: Calculate write offsets
    // Not implemented - Write Offsets... Aren't exactly sure what/if these are
    // used yet.

    ctx->init = INITIALIZED;

    return 0;
}

int oe_close_ctx(oe_ctx ctx) {

    // TODO: Action if close returns -1?
    if (ctx->init == INITIALIZED) {
        close(ctx->header.fid);
        close(ctx->config.fid);
        close(ctx->data.fid);
        close(ctx->signal.fid);
    } else {
        return OE_ENOTINIT;
    }

    return 0;
}

int oe_destroy_ctx(oe_ctx ctx)
{
    if (ctx == NULL)
        return OE_ENULLCTX;

    // Close potentially option streams
    oe_close_ctx(ctx);

    free(ctx->header.path);
    free(ctx->config.path);
    free(ctx->data.path);
    free(ctx->signal.path);
    free(ctx->map.devs);
    free(ctx);

    return 0;
}

int oe_set_ctx_opt(oe_ctx ctx,
                   int option,
                   const void *option_value,
                   size_t option_len)
{

    switch (option) {
        case OE_HEADERSTREAMPATH:
            if (ctx->init == INITIALIZED)
                return OE_ECANTSETOPT;
            ctx->header.path = realloc(ctx->header.path, option_len);
            memcpy(ctx->header.path, option_value, option_len);
            return 0;

        case OE_CONFIGSTREAMPATH:
            if (ctx->init == INITIALIZED)
                return OE_ECANTSETOPT;
            ctx->config.path = realloc(ctx->config.path, option_len);
            memcpy(ctx->config.path, option_value, option_len);
            return 0;

        case OE_DATASTREAMPATH:
            if (ctx->init == INITIALIZED)
                return OE_ECANTSETOPT;
            ctx->data.path = realloc(ctx->data.path, option_len);
            memcpy(ctx->data.path, option_value, option_len);
            return 0;

        case OE_SIGNALSTREAMPATH:
            if (ctx->init == INITIALIZED)
                return OE_ECANTSETOPT;
            ctx->signal.path = realloc(ctx->signal.path, option_len);
            memcpy(ctx->signal.path, option_value, option_len);
            return 0;

        default:
            return OE_EINVALOPT;
    }

    return OE_EINVALARG;
}

// TODO: (pdak) Currently breakable with non-Cstring path. -- Fixed?
// TODO: In case of error, should the true option_len still be set?
int oe_get_ctx_opt(const oe_ctx ctx,
                   int option,
                   void *option_value,
                   size_t *option_len)
{

    switch (option) {
        case OE_HEADERSTREAMPATH:
            if (*option_len >= strlen(ctx->header.path) + 1) {
                size_t n = strlen(ctx->header.path) + 1;
                memcpy(option_value, ctx->header.path, *option_len);
                *option_len = n;
                return 0;
            }
            break;

        case OE_CONFIGSTREAMPATH:
            if (*option_len >= strlen(ctx->config.path) + 1) {
                size_t n = strlen(ctx->config.path) + 1;
                memcpy(option_value, ctx->config.path, n);
                *option_len = n;
                return 0;
            }
            break;

        case OE_DATASTREAMPATH:
            if (*option_len >= strlen(ctx->data.path) + 1) {
                size_t n = strlen(ctx->data.path) + 1;
                memcpy(option_value, ctx->data.path, n);
                *option_len = n;
                return 0;
            }
            break;

        case OE_SIGNALSTREAMPATH:
            if (*option_len >= strlen(ctx->signal.path) + 1) {
                size_t n = strlen(ctx->signal.path) + 1;
                memcpy(option_value, ctx->signal.path, n);
                *option_len = n;
                return 0;
            }
            break;

        case OE_DEVIDS:
            if (*option_len >= sizeof(int) * ctx->map.num_dev) {

                int i;
                for (i = 0; i < ctx->map.num_dev; i++) {
                    memcpy((int *)option_value + i * sizeof(int),
                           &(ctx->map.devs[i].id),
                           sizeof(int));
                    }
                *option_len = sizeof(int) * ctx->map.num_dev;
            }
            break;

        case OE_DEVREADOFFSETS:
            if (*option_len >= sizeof(int) * ctx->map.num_dev) {

                int i;
                for (i = 0; i < ctx->map.num_dev; i++) {
                    memcpy((int *)option_value + i * sizeof(int),
                           &(ctx->map.devs[i].read_offset),
                           sizeof(int));
                    }
                *option_len = sizeof(int) * ctx->map.num_dev;
            }
            break;

        case OE_NUMDEVICES:
            if (*option_len >= sizeof(int)) {
                *(int *)option_value = (ctx->map.num_dev);
                *option_len = sizeof(int);
            }
            break;

        default:
            return OE_EINVALOPT;
            break;
    }

    return OE_EINVALARG;
}

// TODO: Page 31 of xillybus programming manual
// Also, look at OE code
int oe_write_reg(const oe_ctx ctx, int device_idx, int addr, int value, int *ack)
{
    // Checks that the device index is valid
    if (device_idx >= ctx->map.num_dev || device_idx < 0)
        return OE_EDEVIDX;

    if (lseek(ctx->config.fid, 0, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    // Pump the signal stream unil firmware provide config write start
    int sig;
    do {
        int rc = oe_signal_read(ctx, &sig);
        if (rc < 0) return rc;

    } while (sig != OE_CONFIGWSTART);

    // TODO: Replace sequential calls to write() with explict lseek calls to correct
    // registers followed by write. We might need to include these offsets in the
    // header stream? Or just hardcode them.
    write(ctx->config.fid, &device_idx, sizeof(int));
    write(ctx->config.fid, &addr, sizeof(int));
    write(ctx->config.fid, &value, sizeof(int));

    int trig = 0x00000001;
    write(ctx->config.fid, &trig, sizeof(int));

    // Block until we get an ack signal
    read(ctx->signal.fid, &ack, sizeof(int));

    return 0;
}

int oe_read_reg(const oe_ctx ctx, int device_idx, int addr, int *value, int *ack)
{
    // Checks that the device index is valid
    if (device_idx >= ctx->map.num_dev || device_idx < 0)
        return OE_EDEVIDX;
    if (lseek(ctx->config.fid, 0, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    // Pump the signal stream unil firmware provide config read start
    int sig;
    do {
        int rc = oe_signal_read(ctx, &sig);
        if (rc < 0) return rc;

    } while (sig != OE_CONFIGWSTART);

    // TODO: Replace sequential calls to write() with explict lseek calls to correct
    // registers followed by write. We might need to include these offsets in the
    // header stream? Or just hardcode them.
    write(ctx->config.fid, &device_idx, sizeof(int));
    write(ctx->config.fid, &addr, sizeof(int));
    if (lseek(ctx->config.fid, 24, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;
    int trig = 0x00000001;
    write(ctx->config.fid, &trig, sizeof(int));

    if (lseek(ctx->config.fid, 20, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    read(ctx->config.fid, value, sizeof(int));

    // Block until we get an ack signal
    read(ctx->signal.fid, &ack, sizeof(int));

    return 0;
}

int oe_read(const oe_ctx ctx, void *data, size_t size)
{
    return oe_all_read(ctx->data.fid, data, size);
}

// TODO: int oe_write(const oe_ctx* state, void* data, size_t size){}

static int oe_all_read(int fd, void *data, size_t size)
{
    int received = 0;

    while (received < size) {

        int rc = read(fd, (char *)data + received, size - received);

        if ((rc < 0) && (errno == EINTR))
            continue;

        if (rc < 0)
            return OE_EREADFAILURE;

        if (rc == 0)
            return OE_EREADFAILURE;

        received += rc;
    }
    return 0;
}

static int oe_signal_read(const oe_ctx ctx, int *sig)
{
    return oe_all_read(ctx->signal.fid, sig, sizeof(int));
}
