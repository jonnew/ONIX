#include "stdio.h"
#include "errno.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#include "oedevice.h"
#include "oepcie.h"

const char *oe_error_string[]
    = {"Success",
       "Invalid stream path, fail on open",
       "Double initialization attempt",
       "Invalid device ID on init or reg op",
       "Failure to read from a stream/register",
       "Failure to write to a stream/register",
       "Attempt to call function w null ctx",
       "Failure to seek on stream",
       "Invalid operation on non-initialized ctx",
       "Invalid device index",
       "Invalid context option",
       "Invalid function arguments",
       "Option cannot be set in current context state",
       "Invalid COBS packet",
       "Attempt to trigger an already triggered operation",
       "Supplied buffer is too small",
       "Badly formatted device map supplied by firmware",
       "Bad dynamic memory allocation"};

typedef struct stream_fid {
    char* path;
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
    size_t num_dev;
    oe_device_t* dev_map;

    // Acqusition state
    run_state_t run_state;

} oe_ctx_impl_t;

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
    int rc = oe_write_reg(ctx, OEPCIEMASTER, OEPCIEMASTER_HEADER, 0);
    if (rc) return rc;

    // Pump for header start
    rc = _oe_pump_signal_type(ctx->signal.fid, OE_HEADERSTART);
    if (rc) return rc;

    ctx->num_dev = 0;
    while(1) {

        oe_signal_t sig_type = OE_NULLSIG;
        size_t b_size = 255;
        uint8_t buffer[b_size];
        int rc = _oe_read_signal_data(ctx->signal.fid, &sig_type, buffer, &b_size);
        if (rc) return rc;

        // Following a HEADERSTART, these are the only two signals that should
        // appear
        if (sig_type != OE_HEADEREND && sig_type != OE_DEVICEINST)
            return OE_EBADDEVMAP;

        // All done
        if (sig_type == OE_HEADEREND) {
            break;
        } else if (sig_type == OE_DEVICEINST) {

            int32_t device_id = (int32_t)(*buffer);

            if (device_id >= MIN_DEVICE_ID && device_id <= MAX_DEVICE_ID) {

                // Insert space for new device in the map
                ctx->num_dev++;
                oe_device_t *new_map
                    = realloc(ctx->dev_map, ctx->num_dev * sizeof(oe_device_t));
                if (new_map)
                    ctx->dev_map = new_map;
                else
                    return OE_EBADALLOC;

                // Append the device onto the map
                ctx->dev_map[ctx->num_dev - 1] = *(oe_device_t *)buffer;

            } else {
                return OE_EDEVID;
            }

        } else {

            // Following a HEADERSTART, these are the only two signals that
            // should appear
            return OE_EBADDEVMAP;
        }
    }

    return 0;
}

int oe_close_ctx(oe_ctx ctx)
{
    // TODO: Action if close returns -1?
    if (ctx->run_state > UNINITIALIZED) {
        close(ctx->config.fid);
        close(ctx->data.fid);
        close(ctx->signal.fid);
        ctx->run_state = UNINITIALIZED;
    } else {

        return OE_ENOTINIT;
    }

    return 0;
}

int oe_destroy_ctx(oe_ctx ctx)
{
    if (ctx == NULL)
        return OE_ENULLCTX;

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
                   oe_ctx_opt_t option,
                   const void *option_value,
                   size_t option_len)
{
    if (ctx == NULL)
        return OE_ENULLCTX;

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
                   oe_ctx_opt_t option,
                   void *option_value,
                   size_t *option_len)
{

    if (ctx == NULL)
        return OE_ENULLCTX;

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
            size_t required_bytes = sizeof(oe_device_t) * ctx->num_dev;
            if (*option_len >= required_bytes) {

                memcpy(option_value, ctx->dev_map, required_bytes);
                *option_len = required_bytes;
            }
            break;
        }
        case OE_NUMDEVICES: {
            size_t required_bytes = sizeof(oe_size_t);
            if (*option_len >= required_bytes) {
                *(oe_size_t *)option_value = ctx->num_dev;
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

int oe_write_reg(const oe_ctx ctx, int32_t device_idx, uint32_t addr, uint32_t value)
{
    // TODO: This should be an assert, probably
    if (ctx == NULL)
        return OE_ENULLCTX;

    // TODO: This should be an assert, probably
    if (ctx->run_state == UNINITIALIZED)
        return OE_ENOTINIT;

    // Checks that the device index is valid
    // TODO: This should be an assert, probably
    if (device_idx >= ctx->num_dev && device_idx != OEPCIEMASTER)
        return OE_EDEVIDX;

    if (lseek(ctx->config.fid, OE_CONFTRIG, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    // Make sure we are not already in config triggered state
    uint8_t trig = 0x00;
    if (read(ctx->config.fid, &trig, 1) == 0)
        return OE_EREADFAILURE;

    if (trig != 0)
        return OE_ERETRIG;

    // Set config registers and trigger a write
    if (lseek(ctx->config.fid, OE_CONFDEVID, SEEK_SET) < 0)
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

    return _oe_pump_signal_type(ctx->signal.fid, OE_CONFIGWACK);
}

int oe_read_reg(const oe_ctx ctx, int device_idx, uint32_t addr, uint32_t *value)
{
    // TODO: This should be an assert, probably
    if (ctx == NULL)
        return OE_ENULLCTX;

    // TODO: This should be an assert, probably
    if (ctx->run_state == UNINITIALIZED)
        return OE_ENOTINIT;

    // Checks that the device index is valid
    // TODO: This should be an assert, probably
    if (device_idx >= ctx->num_dev && device_idx != OEPCIEMASTER)
        return OE_EDEVIDX;

    if (lseek(ctx->config.fid, OE_CONFTRIG, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    // Make sure we are not already in config triggered state
    uint8_t trig = 0x00;
    if (read(ctx->config.fid, &trig, 1) == 0)
        return OE_EREADFAILURE;

    if (trig > 0)
        return OE_ERETRIG;

    // Set config registers and trigger a write
    if (lseek(ctx->config.fid, OE_CONFDEVID, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    if (write(ctx->config.fid, &device_idx, 4) <= 0)
        return OE_EWRITEFAILURE;

    if (write(ctx->config.fid, &addr, 4) <= 0)
        return OE_EWRITEFAILURE;

    uint8_t rw = 0x00;
    if (write(ctx->config.fid, &rw, 1) <= 0)
        return OE_EWRITEFAILURE;

    trig = 0x01;
    if (write(ctx->config.fid, &trig, 1) <= 0)
        return OE_EWRITEFAILURE;

    // Pump the signal stream until firmware provide config read ack and retreive value
    size_t size = 4;
    return _oe_pump_signal_data(ctx->signal.fid, OE_CONFIGRACK, value, &size);
}

int oe_read(const oe_ctx ctx, void *data, size_t size)
{
    return _oe_read(ctx->data.fid, data, size);
}

static inline int _oe_read(int fd, void *data, size_t size)
{
    int received = 0;

    while (received < size) {

        int rc = read(fd, (char *)data + received, size - received);

        if ((rc < 0) && (errno == EINTR))
            continue;

        if (rc <= 0)
            return OE_EREADFAILURE;

        received += rc;
    }

    return received;
}

void oe_error(oe_error_t err, char *str, size_t str_len)
{
    if (err > OE_MINERRORNUM && err < 0)
        snprintf(str, str_len, "%s", oe_error_string[err]);
    else
        snprintf(str, str_len, "Unknown error %d", err);
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
        return i;
}

static int _oe_read_signal_type(int signal_fd, oe_signal_t *type)
{
    if (type == NULL)
        return OE_EINVALARG;

    uint8_t buffer[255] = {0};

    int pack_size = _oe_read_signal_packet(signal_fd, buffer);
    if (pack_size < 0) return pack_size;

    // Unstuff the packet (last byte is the 0, so we decrement
    int rc = _oe_cobs_unstuff(buffer, buffer, pack_size);
    if (rc < 0) return rc;

    // Get the type, which occupies first 4 bytes of buffer
    *type = *(oe_signal_t *)buffer;

    return 0;
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

static int _oe_pump_signal_type(int signal_fd, const oe_signal_t type)
{
    oe_signal_t packet_type = OE_NULLSIG;
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

    } while (packet_type != type);

    return 0;
}

static int _oe_pump_signal_data(int signal_fd,
                                const oe_signal_t type,
                                void *data,
                                size_t *size)
{
    oe_signal_t packet_type = OE_NULLSIG;
    int pack_size = 0;
    uint8_t buffer[255];

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
    } while (packet_type != type);

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
    // Minimal COBS packet is 1 byte + delimiter
    // Maximal COBS packet is 254 bytes + delimiter
    if (size < 2 || size > 255)
        return OE_ECOBSPACK;

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
