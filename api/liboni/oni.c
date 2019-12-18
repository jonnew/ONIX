// Hint -- "NB:..." indicates why. Other comments indicate what.
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oni.h"
#include "onidriverloader.h"

// Define if hardware produces byte-reversed types from compile command
#ifdef ONI_BIGEND

// NB: Nested defines allows compile command to define ONI_BE, etc.
#define ONI_BE

#define BSWAP_32(val)                                                          \
     ( (((uint32_t)(val)&0x000000ff) << 24)                                    \
     | (((uint32_t)(val)&0x0000ff00) << 8)                                     \
     | (((uint32_t)(val)&0x00ff0000) >> 8)                                     \
     | (((uint32_t)(val)&0xff000000) >> 24))

#endif

// Consistent overhead bytestuffing buffer size
#define ONI_COBSBUFFERSIZE 255

// Minimal high-bandwidth fifo read size (bytes)
#define ONI_DATAFIFOWIDTH 4

// Reference counter
struct ref {
    void (*free)(const struct ref *);
    int count;
};

// Reference-counted read buffer
struct oni_buf_impl {

    // Raw data buffer
    uint8_t *buffer;
    uint8_t *read_pos;
    uint8_t *end_pos;

    // Reference count
    struct ref count;
};

// Acqusition context
struct oni_ctx_impl {

    oni_driver_t driver;

    // Run acqusition immediately following reset
    oni_reg_val_t run_on_reset;

    // Devices
    oni_size_t num_dev;
    oni_device_t* dev_map;

    // Maximum read frame size (bytes, includes header)
    oni_size_t max_read_frame_size;

    // Block read size (bytes, defaults to max_read_frame_size)
    oni_size_t block_read_size;

    // Current, attached buffer
    struct oni_buf_impl *shared_buf;

    // Acqusition state
    enum {
        CTXNULL = 0,
        UNINITIALIZED,
        IDLE,
        RUNNING
    } run_state;
};

// Signal flags
typedef enum oni_signal {
    NULLSIG             = (1u << 0),
    CONFIGWACK          = (1u << 1), // Configuration write-acknowledgment
    CONFIGWNACK         = (1u << 2), // Configuration no-write-acknowledgment
    CONFIGRACK          = (1u << 3), // Configuration read-acknowledgment
    CONFIGRNACK         = (1u << 4), // Configuration no-read-acknowledgment
    DEVICEMAPACK        = (1u << 5), // Device map start acknowledgment
    DEVICEINST          = (1u << 6), // Device map instance
} oni_signal_t;

// Static helpers
static int _oni_reset_routine(oni_ctx ctx);
static inline int _oni_read(oni_ctx ctx, oni_read_stream_t stream, void* data, size_t size);
static inline int _oni_write(oni_ctx ctx, oni_write_stream_t stream, const char* data, size_t size);
static int _oni_read_signal_packet(oni_ctx ctx, uint8_t *buffer);
static int _oni_read_signal_data(oni_ctx ctx, oni_signal_t *type, void *data, size_t size);
static int _oni_pump_signal_type(oni_ctx ctx, int flags, oni_signal_t *type);
static int _oni_pump_signal_data(oni_ctx ctx, int flags, oni_signal_t *type, void *data, size_t size);
static int _oni_cobs_unstuff(uint8_t *dst, const uint8_t *src, size_t size);
static inline int _oni_write_config(oni_ctx ctx, oni_config_t reg, oni_reg_val_t value);
static inline int _oni_read_config(oni_ctx, oni_config_t reg, oni_reg_val_t *value);
static int _oni_read_buffer(oni_ctx ctx, void **data, size_t size, int);
static void _oni_destroy_buffer(const struct ref *ref);
static inline void _ref_inc(const struct ref *ref);
static inline void _ref_dec(const struct ref *ref);
#ifdef ONI_BE
static int _device_map_byte_swap(oni_ctx ctx);
#endif

oni_ctx oni_create_ctx(const char* drv_name)
{
    oni_ctx ctx = calloc(1, sizeof(struct oni_ctx_impl));

    if (ctx == NULL) {
        errno = EAGAIN;
        return NULL;
    }

    if (oni_create_driver(drv_name, &ctx->driver))
    {
        errno = EINVAL;
        free(ctx);
        return NULL;
    }

    ctx->num_dev = 0;
    ctx->dev_map = NULL;
    ctx->run_state = UNINITIALIZED;
    ctx->run_on_reset = 0;

    return ctx;
}

int oni_init_ctx(oni_ctx ctx, int host_idx)
{
    assert(ctx != NULL && "Context is NULL.");
    assert(ctx->run_state == UNINITIALIZED && "Context is in invalid state.");

    if (ctx->run_state != UNINITIALIZED)
        return ONI_EINVALSTATE;

    int rc = ctx->driver.init(ctx->driver.ctx, host_idx);
    if (rc) return rc;

    // NB: Trigger reset routine (populates device map and key acqusition
    // parameters) Success will set run_state to IDLE

    // If running, stop acqusition
    rc = _oni_write_config(ctx, ONI_CONFIG_RUNNING, ctx->run_on_reset);
    if (rc) return rc;

    // Set the reset register
    rc = _oni_write_config(ctx, ONI_CONFIG_RESET, 1);
    if (rc) return rc;

    // Get device map etc
    rc = _oni_reset_routine(ctx);
    if (rc) return rc;

    // Run state is now IDLE
    ctx->run_state = IDLE;

    return ONI_ESUCCESS;
}

int oni_destroy_ctx(oni_ctx ctx)
{
    assert(ctx != NULL && "Context is NULL");
    int rc = ctx->driver.destroy_ctx(ctx->driver.ctx);
    if (rc) return rc;

    free(ctx->dev_map);
    free(ctx);

    return ONI_ESUCCESS;
}

int oni_get_opt(const oni_ctx ctx, int ctx_opt, void *value, size_t *option_len)
{
    assert(ctx != NULL && "Context is NULL");

    switch (ctx_opt) {
        case ONI_DEVICEMAP: {

            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return ONI_EINVALSTATE;

            size_t required_bytes = sizeof(oni_device_t) * ctx->num_dev;
            if (*option_len < required_bytes)
                return ONI_EBUFFERSIZE;

            memcpy(value, ctx->dev_map, required_bytes);
            *option_len = required_bytes;
            break;
        }
        case ONI_NUMDEVICES: {

            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return ONI_EINVALSTATE;

            size_t required_bytes = sizeof(oni_size_t);
            if (*option_len < required_bytes)
                return ONI_EBUFFERSIZE;

            *(oni_size_t *)value = ctx->num_dev;
            *option_len = required_bytes;
            break;
        }
        case ONI_MAXREADFRAMESIZE: {

            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return ONI_EINVALSTATE;

            size_t required_bytes = sizeof(oni_size_t);
            if (*option_len < required_bytes)
                return ONI_EBUFFERSIZE;

            *(oni_size_t *)value = ctx->max_read_frame_size;
            *option_len = required_bytes;
            break;
        }
        case ONI_RUNONRESET: {

            if (*option_len != sizeof(oni_reg_val_t))
                return ONI_EBUFFERSIZE;

            *(oni_reg_val_t *)value = ctx->run_on_reset;
            *option_len = sizeof(oni_reg_val_t);
            break;
        }
        case ONI_RUNNING: {

            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return ONI_EINVALSTATE;

            if (*option_len != ONI_REGSZ)
                return ONI_EBUFFERSIZE;

            int rc = _oni_read_config(ctx, ONI_CONFIG_RUNNING, value);
            if (rc) return rc;

            *option_len = ONI_REGSZ;
            break;
        }
        case ONI_RESET: {

            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return ONI_EINVALSTATE;

            if (*option_len != ONI_REGSZ)
                return ONI_EBUFFERSIZE;

            int rc = _oni_read_config(ctx, ONI_CONFIG_RESET, value);
            if (rc) return rc;

            *option_len = ONI_REGSZ;
            break;
        }
        case ONI_SYSCLKHZ: {

            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return ONI_EINVALSTATE;

            if (*option_len != ONI_REGSZ)
                return ONI_EBUFFERSIZE;

            int rc = _oni_read_config(ctx, ONI_CONFIG_SYSCLK, value);
            if (rc) return rc;

            *option_len = ONI_REGSZ;
            break;
        }
        case ONI_BLOCKREADSIZE: {
            oni_size_t required_bytes = sizeof(oni_size_t);
            if (*option_len < required_bytes)
                return ONI_EBUFFERSIZE;

            *(oni_size_t *)value = ctx->block_read_size;
            *option_len = required_bytes;
            break;
        }
        default:
            return ONI_EINVALOPT;
    }

    return ONI_ESUCCESS;
}

int oni_set_opt(oni_ctx ctx, int ctx_opt, const void *value, size_t option_len)
{
    assert(ctx != NULL && "Context is NULL");

    switch (ctx_opt) {
        case ONI_RUNONRESET: {
            // Can be set in any context state
            if (option_len != sizeof(oni_reg_val_t))
                return ONI_EBUFFERSIZE;

            ctx->run_on_reset =  *(oni_reg_val_t *)value;
            break;
        }
        case ONI_RUNNING: {
            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return ONI_EINVALSTATE;

            if (option_len != ONI_REGSZ)
                return ONI_EBUFFERSIZE;

            int rc = _oni_write_config(
                ctx, ONI_CONFIG_RUNNING, *(oni_reg_val_t*)value);
            if (rc) return rc;

            if (*(oni_reg_val_t *)value != 0)
                ctx->run_state = RUNNING;
            else
                ctx->run_state = IDLE;
            break;
        }
        case ONI_RESET: {
            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state != IDLE)
                return ONI_EINVALSTATE;

            if (option_len != ONI_REGSZ)
                return ONI_EBUFFERSIZE;

            if (*(oni_reg_val_t *)value != 0) {

                // Set the reset register
                int rc = _oni_write_config(
                    ctx, ONI_CONFIG_RESET, *(oni_reg_val_t*)value);
                if (rc) return rc;

                // Get device map etc
                _oni_reset_routine(ctx);
            }

            break;
        }
        case ONI_SYSCLKHZ: {
            return ONI_EREADONLY;
        }
        case ONI_BLOCKREADSIZE: {
            // NB: If we are careful, this could be changed during RUNNING
            // state. However, I would need to perform runtime size check of
            // the block readsize. Also, what if it occured on a separate
            // thread during a call to oni_read_frame?
            assert(ctx->run_state == IDLE && "Context state must be IDLE.");
            if (ctx->run_state != IDLE)
                return ONI_EINVALSTATE;

            if (option_len != sizeof(oni_size_t))
                return ONI_EBUFFERSIZE;

            oni_size_t block_read_size = *(oni_size_t *)value;

            // Make sure the block read size is greater than max frame size
            if (block_read_size < ctx->max_read_frame_size)
                return ONI_EINVALREADSIZE;

            // Make sure the block read size is a multiple of the FIFO width
            if (block_read_size % ONI_DATAFIFOWIDTH != 0)
                return ONI_EINVALREADSIZE;

            ctx->block_read_size = block_read_size;

            break;

        }
        default:
            return ONI_EINVALOPT;
    }

    return ctx->driver.set_opt_callback(ctx->driver.ctx, ctx_opt, value, option_len);
}

int oni_get_driver_opt(const oni_ctx ctx, int drv_opt, void* value, size_t *option_len)
{
    return ctx->driver.get_opt(ctx->driver.ctx, drv_opt, value, option_len);
}

int oni_set_driver_opt(oni_ctx ctx, int drv_opt, const void* value, size_t option_len)
{
    return ctx->driver.set_opt(ctx->driver.ctx, drv_opt, value, option_len);
}

int oni_write_reg(const oni_ctx ctx,
                  size_t dev_idx,
                  oni_reg_addr_t addr,
                  oni_reg_val_t value)
{
    assert(ctx != NULL && "Context is NULL");
    assert(ctx->run_state > UNINITIALIZED && "Context must be INITIALIZED.");

    // Checks that the device index is valid
    if (dev_idx >= ctx->num_dev && dev_idx)
        return ONI_EDEVIDX;

    // Make sure we are not already in config triggered state
    oni_reg_val_t trig = 0;
    int rc = _oni_read_config(ctx, ONI_CONFIG_TRIG, &trig);
    if (rc) return rc;

    if (trig != 0) return ONI_ERETRIG;

    // Set config registers and trigger a write
    rc = _oni_write_config(ctx, ONI_CONFIG_DEVICE_IDX, dev_idx);
    if (rc) return rc;
    rc = _oni_write_config(ctx, ONI_CONFIG_REG_ADDR, addr);
    if (rc) return rc;
    rc = _oni_write_config(ctx, ONI_CONFIG_REG_VALUE, value);
    if (rc) return rc;

    oni_reg_val_t rw = 1;
    rc = _oni_write_config(ctx, ONI_CONFIG_RW, rw);
    if (rc) return rc;

    trig = 1;
    rc = _oni_write_config(ctx, ONI_CONFIG_TRIG, trig);
    if (rc) return rc;

    // Wait for reponse from hardware
    oni_signal_t type;
    rc = _oni_pump_signal_type(ctx, CONFIGWACK | CONFIGWNACK, &type);
    if (rc) return rc;

    if (type == CONFIGWNACK) return ONI_EWRITEFAILURE;

    return ONI_ESUCCESS;
}

int oni_read_reg(const oni_ctx ctx,
                 size_t dev_idx,
                 oni_reg_addr_t addr,
                 oni_reg_val_t *value)
{
    assert(ctx != NULL && "Context is NULL");
    assert(ctx->run_state > UNINITIALIZED && "Context must be INITIALIZED.");

    // Checks that the device index is valid
    if (dev_idx >= ctx->num_dev && dev_idx)
        return ONI_EDEVIDX;

    // Make sure we are not already in config triggered state
    oni_reg_val_t trig = 0;
    int rc = _oni_read_config(ctx, ONI_CONFIG_TRIG, &trig);
    if (rc) return rc;

    if (trig != 0) return ONI_ERETRIG;

    // Set config registers and trigger a write
    rc = _oni_write_config(ctx, ONI_CONFIG_DEVICE_IDX, dev_idx);
    if (rc) return rc;
    rc = _oni_write_config(ctx, ONI_CONFIG_REG_ADDR, addr);
    if (rc) return rc;

    oni_reg_val_t rw = 0;
    rc = _oni_write_config(ctx, ONI_CONFIG_RW, rw);
    if (rc) return rc;

    trig = 1;
    rc = _oni_write_config(ctx, ONI_CONFIG_TRIG, trig);
    if (rc) return rc;

    // Wait for reponse from hardware
    oni_signal_t type;
    rc = _oni_pump_signal_type(ctx, CONFIGRACK | CONFIGRNACK, &type);
    if (rc) return rc;

    if (type == CONFIGRNACK) return ONI_EREADFAILURE;

    rc = _oni_read_config(ctx, ONI_CONFIG_REG_VALUE, value);
    if (rc) return rc;

    return ONI_ESUCCESS;
}

int oni_read_frame(const oni_ctx ctx, oni_frame_t **frame)
{
    assert(ctx != NULL && "Context is NULL");

    // NB: We don't need run_state == RUNNING because this could be changed in
    // a different thread
    assert(ctx->run_state >= IDLE && "Context is not acquiring.");

    if (ctx->max_read_frame_size == 0)
        return ONI_ENOREADDEV;

    // TODO: There is no need for all of this. The header fields in oni_frame_t
    // should just point into the shared buffer.

    // Get the header and figure out how many devices are in the frame
    uint8_t *header = NULL;
    int rc = _oni_read_buffer(ctx, (void **)&header, ONI_RFRAMEHEADERSZ, 1);
    if (rc != 0) return rc;

    // Allocate space for frame container and device offset list
    uint16_t num_dev = *(uint16_t *)(header + sizeof(uint64_t));
    *frame = malloc(sizeof(oni_frame_t) + num_dev * sizeof(oni_size_t));
    oni_frame_t *fptr = *frame;

    // Device offset list points to extra space after frame container
    fptr->dev_offs = (oni_size_t *)((char *)fptr + sizeof(oni_frame_t));

    // Total frame size
    int total_size = sizeof(oni_frame_t);

    // Copy frame header members (contiuous)
    // 1. clock (8)
    // 2. n_dev (2)
    // 3. corrupt (1)
    memcpy(&fptr->clock, header, sizeof(uint64_t) + sizeof(uint16_t) + sizeof(uint8_t));

    // Read device indices that are in this frame
    rc = _oni_read_buffer(
        ctx, (void **)&fptr->dev_idxs, fptr->num_dev * sizeof(oni_size_t), 0);
    if (rc != 0) return rc;

    // Find data read size
    uint16_t i;
    int rsize = 0;
    for (i = 0; i < fptr->num_dev; i++) {
        *(fptr->dev_offs + i) = rsize;
        rsize += ctx->dev_map[*(fptr->dev_idxs + i)].read_size;
    }

    // Find read size (+ padding)
    rsize += rsize % ONI_DATAFIFOWIDTH;
    fptr->data_sz = rsize;
    total_size += rsize;

    // Read data
    rc = _oni_read_buffer(ctx, (void **)&fptr->data, rsize, 0);
    if (rc != 0) return rc;

    // Update buffer ref count and provide reference to frame
    _ref_inc(&(ctx->shared_buf->count));
    fptr->buffer = ctx->shared_buf;

     return total_size;
}

int oni_write(const oni_ctx ctx, size_t dev_idx, const void *data, size_t data_sz)
{
    assert(ctx != NULL && "Context is NULL");

    // NB: We don't need run_state == RUNNING because this could be changed in
    // a different thread
    assert(ctx->run_state >= IDLE && "Context is not acquiring.");

    // Checks that the device index is valid
    if (dev_idx >= ctx->num_dev)
        return ONI_EDEVIDX;

    // Get the device from the map
    oni_device_t *this_dev = ctx->dev_map + dev_idx;

    // Make sure this device is writable and write size is correct
    if (this_dev->write_size == 0 || data_sz != this_dev->write_size)
        return ONI_EWRITESIZE;

    // TODO: Seems unncessary to copy here but if I dont, I perform two write calls
    size_t wsize = data_sz + sizeof(oni_size_t);
    wsize += wsize % ONI_DATAFIFOWIDTH;
    char *buffer = malloc(wsize);
    memcpy(buffer, (oni_size_t *)&dev_idx, sizeof(oni_size_t));
    memcpy(buffer + sizeof(oni_size_t), data, data_sz);

    int rc = _oni_write(ctx, ONI_WRITE_STREAM_DATA, buffer, wsize);

    free(buffer);

    if (rc != (int)wsize) return ONI_EWRITEFAILURE;

    return rc;
}

void oni_destroy_frame(oni_frame_t *frame)
{
    if (frame != NULL) {

        // Decrement buffer reference count
        _ref_dec(&(frame->buffer->count));

        // Free the container
        free(frame);
    }
}

void oni_version(int *major, int *minor, int *patch)
{
    *major = ONI_VERSION_MAJOR;
    *minor = ONI_VERSION_MINOR;
    *patch = ONI_VERSION_PATCH;
}

const char *oni_error_str(int err)
{
    assert(err > ONI_MINERRORNUM && "Invalid error number.");
    assert(err <= 0 && "Invalid error number.");

    switch (err) {
        case ONI_ESUCCESS: {
            return "Success";
        }
        case ONI_EPATHINVALID: {
            return "Invalid stream path";
        }
        case ONI_EDEVID: {
            return "Invalid device ID";
        }
        case ONI_EDEVIDX: {
            return "Invalid device index";
        }
        case ONI_EWRITESIZE: {
            return "Data write size is incorrect for designated device";
        }
        case ONI_EREADFAILURE: {
            return "Failure to read from a stream or register";
        }
        case ONI_EWRITEFAILURE: {
            return "Failure to write to a stream or register";
        }
        case ONI_ENULLCTX: {
            return "Null context";
        }
        case ONI_ESEEKFAILURE: {
            return "Failure to seek on stream";
        }
        case ONI_EINVALSTATE: {
            return "Invalid operation for the current context run state";
        }
        case ONI_EINVALOPT: {
            return "Invalid context option";
        }
        case ONI_EINVALARG: {
            return "Invalid function arguments";
        }
        case ONI_ECOBSPACK: {
            return "Invalid COBS packet";
        }
        case ONI_ERETRIG: {
            return "Attempt to trigger an already triggered operation";
        }
        case ONI_EBUFFERSIZE: {
            return "Supplied buffer is too small";
        }
        case ONI_EBADDEVMAP: {
            return "Badly formatted device map supplied by firmware";
        }
        case ONI_EBADALLOC: {
            return "Bad dynamic memory allocation";
        }
        case ONI_ECLOSEFAIL: {
            return "File descriptor close failure (check errno)";
        }
        case ONI_EREADONLY: {
            return "Attempted write to read only object (register, context "
                   "option, etc)";
        }
        case ONI_EUNIMPL: {
            return "Unimplemented API feature";
        }
        case ONI_EINVALREADSIZE: {
            return "Block read size is smaller than the maximal frame size";
        }
        case ONI_ENOREADDEV: {
            return "Frame read attempted when there are no readable devices in the device map";
        }
        default:
            return "Unknown error";
    }
}

static int _oni_reset_routine(oni_ctx ctx) {

    // Get number of devices
    oni_signal_t sig_type = NULLSIG;
    int rc = _oni_pump_signal_data(
        ctx, DEVICEMAPACK, &sig_type, &(ctx->num_dev), sizeof(ctx->num_dev));
#ifdef ONI_BE
    ctx->num_dev = BSWAP_32(ctx->num_dev);
#endif
    if (rc) return rc;

    // Make space for the device map
    oni_device_t *new_map
        = realloc(ctx->dev_map, ctx->num_dev * sizeof(oni_device_t));
    if (new_map)
        ctx->dev_map = new_map;
    else
        return ONI_EBADALLOC;

    oni_size_t i;
    ctx->max_read_frame_size = ONI_RFRAMEHEADERSZ;
    size_t total_write_sz = 0;
    for (i = 0; i < ctx->num_dev; i++) {

        sig_type = NULLSIG;
        uint8_t buffer[ONI_COBSBUFFERSIZE];
        rc = _oni_read_signal_data(ctx, &sig_type, buffer, ONI_COBSBUFFERSIZE);
        if (rc) return rc;

        // We should see num_dev device instances appear on the signal stream
        if (sig_type != DEVICEINST)
            return ONI_EBADDEVMAP;

        // Append the device onto the map
        memcpy(ctx->dev_map + i, buffer, sizeof(oni_device_t));
#ifdef ONI_BE
        ctx->max_read_frame_size += BSWAP_32((ctx->dev_map + i)->read_size);
        total_write_sz += BSWAP_32((ctx->dev_map + i)->write_size);
#else
        ctx->max_read_frame_size += (ctx->dev_map + i)->read_size;
        total_write_sz += (ctx->dev_map + i)->write_size;
#endif
    }

    // Fail if there are no devices to control or acquire from
    if (ctx->max_read_frame_size == ONI_RFRAMEHEADERSZ && total_write_sz == 0)
        return ONI_EBADDEVMAP;

    // NB: Default the block read size to a single max sized frame. This is bad
    // for high bandwidth performance and good for closed-loop delay.
    ctx->block_read_size += ctx->max_read_frame_size + ctx->max_read_frame_size % ONI_DATAFIFOWIDTH;

#ifdef ONI_BE
    _device_map_byte_swap(ctx);
#endif

    return ONI_ESUCCESS;
}

static inline int _oni_read(oni_ctx ctx, oni_read_stream_t stream, void *data, size_t size)
{
    return ctx->driver.read_stream(ctx->driver.ctx, stream, data, size);
}

static inline int _oni_write(oni_ctx ctx, oni_write_stream_t stream, const char *data, size_t size)
{
    return ctx->driver.write_stream(ctx->driver.ctx, stream, data, size);
}

static inline int _oni_read_signal_packet(oni_ctx ctx, uint8_t *buffer)
{
    // Read the next zero-deliminated packet
    int i = 0;
    uint8_t curr_byte = 1;
    int bad_delim = 0;
    while (curr_byte != 0) {
        int rc = _oni_read(ctx, ONI_READ_STREAM_SIGNAL, &curr_byte, 1);
        if (rc != 1) return rc;

        if (i < 255)
            buffer[i] = curr_byte;
        else
            bad_delim = 1;

        i++;
    }

    if (bad_delim)
        return ONI_ECOBSPACK;
    else
        return --i; // Length of packet without 0 delimeter
}

static int _oni_read_signal_data(oni_ctx ctx, oni_signal_t *type, void *data, size_t size)
{
    if (type == NULL)
        return ONI_EINVALARG;

    uint8_t buffer[255] = {0};

    int pack_size = _oni_read_signal_packet(ctx, buffer);
    if (pack_size < 0) return pack_size;

    // Unstuff the packet
    int rc = _oni_cobs_unstuff(buffer, buffer, pack_size);
    if (rc < 0) return rc;

    // Remove the overhead byte and signal type
    // and make sure the buffer size is sufficient
    if ((int)size < (--pack_size - 4))
        return ONI_EBUFFERSIZE;

    // Get the type, which occupies first 4 bytes of buffer
    memcpy(type, buffer, sizeof(oni_signal_t));
#ifdef ONI_BE
    *type = BSWAP_32(*type);
#endif

    // pack_size still has overhead byte and header, so we remove those
    size_t data_size = pack_size - sizeof(oni_signal_t);
    if (size < data_size)
        return ONI_EBUFFERSIZE;

    // Copy remaining data into data buffer and update size to reflect the
    // actual data payload size
    memcpy(data, buffer + sizeof(oni_signal_t), data_size);

    return ONI_ESUCCESS;
}

static int _oni_pump_signal_type(oni_ctx ctx, int flags, oni_signal_t *type)
{
    oni_signal_t packet_type = NULLSIG;
    uint8_t buffer[255] = {0};

    do {
        int pack_size = _oni_read_signal_packet(ctx, buffer);

        if (pack_size < 1)
            continue; // Something wrong with delimiter, try again

        // Unstuff the packet (last byte is the 0, so we decrement
        int rc = _oni_cobs_unstuff(buffer, buffer, pack_size);
        if (rc < 0)
            continue; // Something wrong with packet, try again

        // Get the type, which occupies first 4 bytes of buffer
        memcpy(&packet_type, buffer, sizeof(oni_signal_t));
#ifdef ONI_BE
        packet_type = BSWAP_32(packet_type);
#endif

    } while (!(packet_type & flags));

    *type = packet_type;

    return ONI_ESUCCESS;
}

static int _oni_pump_signal_data(
    oni_ctx ctx, int flags, oni_signal_t *type, void *data, size_t size)
{
    oni_signal_t packet_type = NULLSIG;
    int pack_size = 0;
    uint8_t buffer[255] = {0};

    do {
        pack_size = _oni_read_signal_packet(ctx, buffer);

        if (pack_size < 1)
            continue; // Something wrong with delimiter, try again

        // Unstuff the packet (last byte is the 0, so we decrement
        int rc = _oni_cobs_unstuff(buffer, buffer, pack_size);
        if (rc < 0)
            continue;

        // Get the type, which occupies first 4 bytes of buffer
        memcpy(&packet_type, buffer, sizeof(oni_signal_t));
#ifdef ONI_BE
        packet_type = BSWAP_32(packet_type);
#endif

    } while (!(packet_type & flags));

    *type = packet_type;

    // pack_size still has overhead byte and header, so we remove those
    size_t data_size = pack_size - 1 - sizeof(oni_signal_t);
    if (size < data_size)
        return ONI_EBUFFERSIZE;

    // Copy remaining data into data buffer and update size to reflect the
    // actual data payload size
    memcpy(data, buffer + sizeof(oni_signal_t), data_size);

    return ONI_ESUCCESS;
}

static int _oni_cobs_unstuff(uint8_t *dst, const uint8_t *src, size_t size)
{
    // Minimal COBS packet is 1 overhead byte + 1 data byte
    // Maximal COBS packet is 1 overhead byte + 254 data bytes
    assert(size >= 2 && size <= 255 && "Invalid COBS packet buffer size.");

    const uint8_t *end = src + size;
    while (src < end) {
        int code = *src++;
        int i;
        for (i = 1; src < end && i < code; i++)
            *dst++ = *src++;
        if (code < 0xFF)
            *dst++ = 0;
    }

    return ONI_ESUCCESS;
}

static inline int _oni_write_config(oni_ctx ctx,
                            oni_config_t reg,
                            oni_reg_val_t value)
{
    return ctx->driver.write_config(ctx->driver.ctx, reg, value);
}

static inline int _oni_read_config(oni_ctx ctx,
                           oni_config_t reg,
                           oni_reg_val_t *value)
{
    return ctx->driver.read_config(ctx->driver.ctx, reg, value);
}

static int _oni_read_buffer(oni_ctx ctx, void **data, size_t size, int allow_refill)
{
    // Remaining bytes in buffer
    size_t remaining;
    if (ctx->shared_buf != NULL)
        remaining = ctx->shared_buf->end_pos - ctx->shared_buf->read_pos;
    else
        remaining = 0;

    // TODO: Is there a way to get rid of allow_refill?
    // NB: Frames must reference a single buffer, so we must refill if less
    // than max possible frame size on the first read within oni_read_frame().
    // Making this limit smaller will result in memory corruption, so don't do it.
    // Allowing refills multiple times during one call to oni_read_frame() will
    // also cause memory corruption.
    if (remaining < ctx->max_read_frame_size && allow_refill) {

        assert(ctx->max_read_frame_size <= ctx->block_read_size &&
            "Block read size is too small given the possible frame size.");

        // New buffer allocated, old_buffer saved
        struct oni_buf_impl *old_buffer = ctx->shared_buf;
        ctx->shared_buf = malloc(sizeof(struct oni_buf_impl));

        // Allocate data block in buffer
        ctx->shared_buf->buffer = malloc(remaining + ctx->block_read_size);

        // Transfer remaining data to new buffer
        if (old_buffer != NULL) {

            // Copy remaining contents into new buffer
            memcpy(ctx->shared_buf->buffer, old_buffer->read_pos, remaining);

            // Context releases control of old buffer
            _ref_dec(&(old_buffer->count));
        }

        // (Re)set buffer state
        ctx->shared_buf->count = (struct ref) {_oni_destroy_buffer, 1};
        ctx->shared_buf->read_pos = ctx->shared_buf->buffer;
        ctx->shared_buf->end_pos
            = ctx->shared_buf->buffer + remaining + ctx->block_read_size;

        // Fill the buffer with new data
        int rc = _oni_read(ctx, ONI_READ_STREAM_DATA,
                          ctx->shared_buf->buffer + remaining,
                          ctx->block_read_size);
        if ((size_t)rc != ctx->block_read_size) return rc;
    }

    // "Read" (reference) buffer and update buffer read position
    *data = ctx->shared_buf->read_pos;
    ctx->shared_buf->read_pos += size;

    return ONI_ESUCCESS;
}

// NB: Stolen from Linux kernel. Used to get the buffer holding a given
// reference count for buffer freeing.
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

static void _oni_destroy_buffer(const struct ref *ref)
{
    struct oni_buf_impl *buf = container_of(ref, struct oni_buf_impl, count);
    free(buf->buffer);
    free(buf);
}

static inline void _ref_inc(const struct ref *ref)
{
#ifdef _WIN32
    _InterlockedIncrement((int *)&ref->count);
#else
    __sync_add_and_fetch((int *)&ref->count, 1);
#endif
}

static inline void _ref_dec(const struct ref *ref)
{
#ifdef _WIN32
    if (_InterlockedDecrement((int *)&ref->count) == 0)
#else
    if (__sync_sub_and_fetch((int *)&ref->count, 1) == 0)
#endif
        ref->free(ref);
}

#ifdef ONI_BE
static int _device_map_byte_swap(oni_ctx ctx)
{
    size_t i;
    for (i = 0; i < ctx->num_dev; i++) {
        ctx->dev_map[i].id = BSWAP_32(ctx->dev_map[i].id);
        ctx->dev_map[i].read_size = BSWAP_32(ctx->dev_map[i].read_size);
        ctx->dev_map[i].num_reads = BSWAP_32(ctx->dev_map[i].num_reads);
        ctx->dev_map[i].write_size = BSWAP_32(ctx->dev_map[i].write_size);
        ctx->dev_map[i].num_writes = BSWAP_32(ctx->dev_map[i].num_writes);
    }

    return ONI_ESUCCESS;
}

#endif
