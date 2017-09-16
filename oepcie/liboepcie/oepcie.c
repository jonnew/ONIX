#include "assert.h"
#include "errno.h"
#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#include "oepcie.h"

const char *oe_device_string[]
    = {"Invalid", "SERDES-GPIO", "RHD2032", "RHD2064", "MPU9250"};

const char *oe_error_string[]
    = {"Success",
       "Invalid stream path, fail on open",
       "Double initialization attempt",
       "Invalid device ID",
       "Failure to read from a stream/register",
       "Failure to write to a stream/register",
       "Attempt to call function with null context",
       "Failure to seek on stream",
       "Invalid operation on non-initialized context",
       "Invalid device index",
       "Invalid context option",
       "Invalid function arguments",
       "Option cannot be set in current context state",
       "Invalid COBS packet",
       "Attempt to trigger an already triggered operation",
       "Supplied buffer is too small",
       "Badly formatted device map supplied by firmware",
       "Bad dynamic memory allocation",
       "File descriptor close failure (check errno)",
       "Invalid underlying data types",
       "Attempted write to read only object (register, context option, etc)"};

typedef struct stream_fid {
    char *path;
    int fid;
} stream_fid_t;

typedef enum run_state {
    INITIALIZED = 0,
    UNINITIALIZED,
    ACQUIRING
} run_state_t;

typedef struct oe_ctx_impl {
    // Communication channels
    stream_fid_t config;
    stream_fid_t data;
    stream_fid_t signal;

    // Devices
    oe_size_t num_dev;
    oe_device_t* dev_map;

    // Frame sizes (bytes)
    oe_size_t read_frame_size;
    oe_size_t write_frame_size;

    // Acqusition state
    run_state_t run_state;

} oe_ctx_impl_t;

typedef enum oe_signal {
    NULLSIG        = (1u << 0),
    CONFIGWACK     = (1u << 1), // Configuration write-acknowledgement
    CONFIGWNACK    = (1u << 2), // Configuration no-write-acknowledgement
    CONFIGRACK     = (1u << 3), // Configuration read-acknowledgement
    CONFIGRNACK    = (1u << 4), // Configuration no-read-acknowledgement
    DEVICEINST     = (1u << 5), // Deivce instance
} oe_signal_t;

typedef enum oe_config_reg_offset {
    CONFDEVIDOFFSET        = 0,   // Configuration device id register byte offset
    CONFADDROFFSET         = 4,   // Configuration register address register byte offset
    CONFVALUEOFFSET        = 8,   // Configuration register value register byte offset
    CONFRWOFFSET           = 12,  // Configuration register read/write register byte offset
    CONFTRIGOFFSET         = 13,  // Configuration read/write trigger register byte offset
} oe_config_reg_offset_t;

// Static helpers
static inline int _oe_read(int data_fd, void* data, size_t size);
static inline int _oe_read_signal_packet(int signal_fd, uint8_t *buffer);
static int _oe_read_signal_data(int signal_fd, oe_signal_t *type, void *data, size_t *size);
static int _oe_pump_signal_type(int signal_fd, int flags, oe_signal_t *type);
static int _oe_pump_signal_data(int signal_fd, int flags, oe_signal_t *type, void *data, size_t *size);
static int _oe_cobs_unstuff(uint8_t *dst, const uint8_t *src, size_t size);

oe_ctx oe_create_ctx()
{
    oe_ctx ctx = calloc(1, sizeof(struct oe_ctx_impl));

    if (ctx == NULL)
        goto oe_create_ctx_err;

    ctx->config.path = calloc(0, sizeof(char));
    if (ctx->config.path == NULL)
        goto oe_create_ctx_err;

    ctx->data.path = calloc(0, sizeof(char));
    if (ctx->data.path == NULL)
        goto oe_create_ctx_err;

    ctx->signal.path = calloc(0, sizeof(char));
    if (ctx->signal.path == NULL)
        goto oe_create_ctx_err;

    ctx->num_dev = 0;
    ctx->dev_map = NULL; // NB: Enables downstream use of realloc()
    ctx->run_state = UNINITIALIZED;

    return ctx;

oe_create_ctx_err:
    errno = EAGAIN;
    return NULL;
}

int oe_init_ctx(oe_ctx ctx)
{
    assert(ctx != NULL && "Context is NULL");

    // Open all data streams
    if (ctx->run_state == INITIALIZED)
        return OE_EREINITCTX;

    ctx->config.fid = open(ctx->config.path, O_RDWR);
    if (ctx->config.fid == -1)
        return OE_EPATHINVALID;

    ctx->data.fid = open(ctx->data.path, O_RDONLY);
    if (ctx->data.fid == -1)
        return OE_EPATHINVALID;

    ctx->signal.fid = open(ctx->signal.path, O_RDONLY);
    if (ctx->signal.fid == -1)
        return OE_EPATHINVALID;

    // We are now initialized, but lack a device map
    ctx->run_state = INITIALIZED;

    // Get device map from signal stream
    int rc = oe_read_reg(ctx, OE_PCIEMASTERDEVIDX, OE_PCIEMASTERDEV_HEADER, &(ctx->num_dev));
    if (rc) return rc;

    // Make space for the device map
    oe_device_t *new_map
        = realloc(ctx->dev_map, ctx->num_dev * sizeof(oe_device_t));
    if (new_map)
        ctx->dev_map = new_map;
    else
        return OE_EBADALLOC;

    int i;
    int max_roff_idx = 0;
    int max_woff_idx = 0;
    for (i= 0; i < ctx->num_dev; i++) {

        oe_signal_t sig_type = NULLSIG;
        size_t b_size = 255;
        uint8_t buffer[b_size];
        int rc = _oe_read_signal_data(ctx->signal.fid, &sig_type, buffer, &b_size);
        if (rc) return rc;

        // We should see num_dev device instances appear on the signal stream
        if (sig_type != DEVICEINST)
            return OE_EBADDEVMAP;

        int32_t device_id = (int32_t)(*buffer);

        if (device_id > OE_MINDEVICEID && device_id < OE_MAXDEVICEID) {
            ctx->dev_map[i] = *(oe_device_t *)buffer; // Append the device onto the map
            if (ctx->dev_map[i].read_offset > ctx->dev_map[max_roff_idx].read_offset)
                max_roff_idx = i;
            if (ctx->dev_map[i].write_offset > ctx->dev_map[max_woff_idx].write_offset)
                max_woff_idx = i;
        } else {
            return OE_EDEVID;
        }
    }

    // Calculate frame size
    ctx->read_frame_size = ctx->dev_map[max_roff_idx].read_offset
                           + ctx->dev_map[max_roff_idx].read_size;
    ctx->write_frame_size = ctx->dev_map[max_woff_idx].write_offset
                            + ctx->dev_map[max_woff_idx].write_size;
    return 0;
}

int oe_close_ctx(oe_ctx ctx)
{
    assert(ctx != NULL && "Context is NULL");

    if (ctx->run_state > UNINITIALIZED) {
        if (close(ctx->config.fid) == -1) goto oe_close_ctx_fail;
        if (close(ctx->data.fid) == -1) goto oe_close_ctx_fail;
        if (close(ctx->signal.fid) == -1) goto oe_close_ctx_fail;
        ctx->run_state = UNINITIALIZED;
    } else {
        return OE_ENOTINIT;
    }

    return 0;

oe_close_ctx_fail:
    return OE_ECLOSEFAIL;
}

int oe_destroy_ctx(oe_ctx ctx)
{
    assert(ctx != NULL && "Context is NULL");

    // Close potentially open streams
    oe_close_ctx(ctx);

    free(ctx->config.path);
    free(ctx->data.path);
    free(ctx->signal.path);
    free(ctx->dev_map);
    free(ctx);

    return 0;
}

int oe_set_ctx_opt(oe_ctx ctx,
                   const oe_ctx_opt_t option,
                   const void *option_value,
                   const size_t option_len)
{
    assert(ctx != NULL && "Context is NULL");

    switch (option) {

        case OE_CONFIGSTREAMPATH:
            if (ctx->run_state == INITIALIZED)
                return OE_ECANTSETOPT;
            ctx->config.path = realloc(ctx->config.path, option_len);
            memcpy(ctx->config.path, option_value, option_len);
            return 0;

        case OE_DATASTREAMPATH:
            if (ctx->run_state == INITIALIZED)
                return OE_ECANTSETOPT;
            ctx->data.path = realloc(ctx->data.path, option_len);
            memcpy(ctx->data.path, option_value, option_len);
            return 0;

        case OE_SIGNALSTREAMPATH:
            if (ctx->run_state == INITIALIZED)
                return OE_ECANTSETOPT;
            ctx->signal.path = realloc(ctx->signal.path, option_len);
            memcpy(ctx->signal.path, option_value, option_len);
            return 0;

        default:
            return OE_EINVALOPT;
    }

    return OE_EINVALARG;
}

// TODO: In case of error, should the true option_len still be set?
int oe_get_ctx_opt(const oe_ctx ctx,
                   const oe_ctx_opt_t option,
                   void *option_value,
                   size_t *option_len)
{
    assert(ctx != NULL && "Context is NULL");

    switch (option) {

        case OE_CONFIGSTREAMPATH: {
            if (*option_len >= strlen(ctx->config.path) + 1) {
                size_t n = strlen(ctx->config.path) + 1;
                memcpy(option_value, ctx->config.path, n);
                *option_len = n;
                return 0;
            }
            break;
        }
        case OE_DATASTREAMPATH: {
            if (*option_len >= strlen(ctx->data.path) + 1) {
                size_t n = strlen(ctx->data.path) + 1;
                memcpy(option_value, ctx->data.path, n);
                *option_len = n;
                return 0;
            }
            break;
        }
        case OE_SIGNALSTREAMPATH: {
            if (*option_len >= strlen(ctx->signal.path) + 1) {
                size_t n = strlen(ctx->signal.path) + 1;
                memcpy(option_value, ctx->signal.path, n);
                *option_len = n;
                return 0;
            }
            break;
        }
        case OE_DEVICEMAP: {
            assert(ctx->run_state == INITIALIZED && "Context must be initialized.");
            size_t required_bytes = sizeof(oe_device_t) * ctx->num_dev;
            if (*option_len >= required_bytes) {

                memcpy(option_value, ctx->dev_map, required_bytes);
                *option_len = required_bytes;
            }
            break;
        }
        case OE_NUMDEVICES: {
            assert(ctx->run_state == INITIALIZED && "Context must be initialized.");
            size_t required_bytes = sizeof(oe_size_t);
            if (*option_len >= required_bytes) {
                *(oe_size_t *)option_value = ctx->num_dev;
                *option_len = required_bytes;
            }
            break;
        }
        case OE_READFRAMESIZE: {
            assert(ctx->run_state == INITIALIZED && "Context must be initialized.");
            size_t required_bytes = sizeof(oe_size_t);
            if (*option_len >= required_bytes) {
                *(oe_size_t *)option_value = ctx->read_frame_size;
                *option_len = required_bytes;
            }
            break;
        }
        case OE_WRITEFRAMESIZE: {
            assert(ctx->run_state == INITIALIZED && "Context must be initialized.");
            size_t required_bytes = sizeof(oe_size_t);
            if (*option_len >= required_bytes) {
                *(oe_size_t *)option_value = ctx->write_frame_size;
                *option_len = required_bytes;
            }
            break;
        }
        default:
            return OE_EINVALOPT;
            break;
    }

    return OE_EINVALARG;
}

int oe_write_reg(const oe_ctx ctx,
                 const oe_dev_idx_t device_idx,
                 const oe_reg_addr_t addr,
                 const oe_reg_val_t value)
{
    assert(ctx != NULL && "Context is NULL");
    assert(ctx->run_state == INITIALIZED && "Context is not initialized.");

    // Checks that the device index is valid
    if (device_idx >= ctx->num_dev && device_idx != OE_PCIEMASTERDEVIDX)
        return OE_EDEVIDX;

    if (lseek(ctx->config.fid, CONFTRIGOFFSET, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    // Make sure we are not already in config triggered state
    uint8_t trig = 0x00;
    if (read(ctx->config.fid, &trig, 1) == 0)
        return OE_EREADFAILURE;

    if (trig != 0)
        return OE_ERETRIG;

    // Set config registers and trigger a write
    if (lseek(ctx->config.fid, CONFDEVIDOFFSET, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    if (write(ctx->config.fid, &device_idx, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    if (write(ctx->config.fid, &addr, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    if (write(ctx->config.fid, &value, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    uint8_t rw = 0x01;
    if (write(ctx->config.fid, &rw, sizeof(uint8_t)) <= 0)
        return OE_EWRITEFAILURE;

    trig = 0x01;
    if (write(ctx->config.fid, &trig, sizeof(uint8_t)) <= 0)
        return OE_EWRITEFAILURE;

    oe_signal_t type;
    int rc = _oe_pump_signal_type(ctx->signal.fid, CONFIGWACK | CONFIGWNACK, &type);
    if (rc) return rc;

    if (type == CONFIGWNACK)
        return OE_EREADONLY;

    return 0;
}

int oe_read_reg(const oe_ctx ctx,
                const oe_dev_idx_t device_idx,
                const oe_reg_addr_t addr,
                oe_reg_val_t *value)
{
    assert(ctx != NULL && "Context is NULL");
    assert(ctx->run_state == INITIALIZED && "Context is not initialized.");

    // Checks that the device index is valid
    // TODO: This should be an assert, perhaps
    if (device_idx >= ctx->num_dev && device_idx != OE_PCIEMASTERDEVIDX)
        return OE_EDEVIDX;

    if (lseek(ctx->config.fid, CONFTRIGOFFSET, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    // Make sure we are not already in config triggered state
    uint8_t trig = 0x00;
    if (read(ctx->config.fid, &trig, 1) == 0)
        return OE_EREADFAILURE;

    if (trig != 0)
        return OE_ERETRIG;

    // Set config registers and trigger a write
    if (lseek(ctx->config.fid, CONFDEVIDOFFSET, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    if (write(ctx->config.fid, &device_idx, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    if (write(ctx->config.fid, &addr, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    if (write(ctx->config.fid, &value, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    uint8_t rw = 0x00;
    if (write(ctx->config.fid, &rw, sizeof(uint8_t)) <= 0)
        return OE_EWRITEFAILURE;

    trig = 0x01;
    if (write(ctx->config.fid, &trig, sizeof(uint8_t)) <= 0)
        return OE_EWRITEFAILURE;

    size_t size = sizeof(*value);
    oe_signal_t type;
    int rc = _oe_pump_signal_data(
        ctx->signal.fid, CONFIGRACK | CONFIGRNACK, &type, value, &size);
    if (rc) return rc;

    if (type == CONFIGRNACK)
        return OE_EREADONLY;

    return 0;
}

int oe_read(const oe_ctx ctx, void *data, size_t size)
{
    assert(ctx != NULL && "Context is NULL");
    assert(ctx->run_state == INITIALIZED && "Context is not initialized.");

    return _oe_read(ctx->data.fid, data, size);
}

void oe_error(oe_error_t err, char *str, size_t str_len)
{
    if (err > OE_MINERRORNUM && err < 0)
        snprintf(str, str_len, "%s", oe_error_string[-err]);
    else
        snprintf(str, str_len, "Unknown error %d", err);
}

int oe_device(oe_device_id_t dev_id, char *str, size_t str_len)
{
    assert(dev_id > OE_MINDEVICEID && dev_id < OE_MAXDEVICEID && "Invalid device ID.");

    if (dev_id <= OE_MINDEVICEID || dev_id >= OE_MAXDEVICEID)
        return OE_EDEVID;

    snprintf(str, str_len, "%s", oe_device_string[dev_id]);
    return 0;
}

static inline int _oe_read(int data_fd, void *data, size_t size)
{
    int received = 0;

    while (received < size) {

        int rc = read(data_fd, (char *)data + received, size - received);

        if ((rc < 0) && (errno == EINTR))
            continue;

        if (rc <= 0)
            return OE_EREADFAILURE;

        received += rc;
    }

    return received;
}

static inline int _oe_read_signal_packet(int signal_fd, uint8_t *buffer)
{
    // Read the next zero-deliminated packet
    int i = 0;
    uint8_t curr_byte = 1;
    int bad_delim = 0;
    while (curr_byte != 0) {
        int rc = _oe_read(signal_fd, &curr_byte, 1);
        if (rc != 1) return rc;

        if (i < 255)
            buffer[i] = curr_byte;
        else
            bad_delim = 1;

        i++;
    }

    if (bad_delim)
        return OE_ECOBSPACK;
    else
        return --i; // Length of packet without 0 delimeter
}

static int _oe_read_signal_data(int signal_fd, oe_signal_t *type, void *data, size_t *size)
{
    if (type == NULL)
        return OE_EINVALARG;

    uint8_t buffer[255] = {0};

    int pack_size = _oe_read_signal_packet(signal_fd, buffer);
    if (pack_size < 0) return pack_size;

    // Unstuff the packet (last byte is the 0, so we decrement
    int rc = _oe_cobs_unstuff(buffer, buffer, pack_size);
    if (rc < 0) return rc;

    if (*size < (--pack_size - 4))
        return OE_EBUFFERSIZE;

    // Get the type, which occupies first 4 bytes of buffer
    *type = *(oe_signal_t *)buffer;

    // pack_size still has overhead byte and header, so we remove those
    size_t data_size = pack_size - 1 - sizeof(oe_signal_t);
    if (*size < data_size)
        return OE_EBUFFERSIZE;
    else
        *size = data_size;

    // Copy remaining data into data buffer and update size to reflect the
    // actual data payload size
    memcpy(data, buffer + sizeof(oe_signal_t), data_size);

    return 0;
}

static int _oe_pump_signal_type(int signal_fd, int flags, oe_signal_t *type)
{
    oe_signal_t packet_type = NULLSIG;
    uint8_t buffer[255] = {0};

    do {
        int pack_size = _oe_read_signal_packet(signal_fd, buffer);

        if (pack_size < 1)
            continue; // Something wrong with delimiter, try again

        // Unstuff the packet (last byte is the 0, so we decrement
        int rc = _oe_cobs_unstuff(buffer, buffer, pack_size);
        if (rc < 0)
            continue; // Something wrong with packet, try again

        // Get the type, which occupies first 4 bytes of buffer
        packet_type = *(oe_signal_t *)buffer;

    } while (!(packet_type & flags));

    *type = packet_type;

    return 0;
}

static int _oe_pump_signal_data(
    int signal_fd, int flags, oe_signal_t *type, void *data,  size_t *size)
{
    oe_signal_t packet_type = NULLSIG;
    int pack_size = 0;
    uint8_t buffer[255] = {0};

    do {
        pack_size = _oe_read_signal_packet(signal_fd, buffer);

        if (pack_size < 1)
            continue; // Something wrong with delimiter, try again

        // Unstuff the packet (last byte is the 0, so we decrement
        int rc = _oe_cobs_unstuff(buffer, buffer, pack_size);
        if (rc < 0)
            continue;

        // Get the type, which occupies first 4 bytes of buffer
        packet_type = *(oe_signal_t *)buffer;

    } while (!(packet_type & flags));

    *type = packet_type;

    // pack_size still has overhead byte and header, so we remove those
    size_t data_size = pack_size - 1 - sizeof(oe_signal_t);
    if (*size < data_size)
        return OE_EBUFFERSIZE;
    else
        *size = data_size;

    // Copy remaining data into data buffer and update size to reflect the
    // actual data payload size
    memcpy(data, buffer + sizeof(oe_signal_t), data_size);

    return 0;
}

static int _oe_cobs_unstuff(uint8_t *dst, const uint8_t *src, size_t size)
{
    // Minimal COBS packet is 1 overhead byte + 1 data byte
    // Maximal COBS packet is 1 overhead byte + 254 datate bytes
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

    return 0;
}
