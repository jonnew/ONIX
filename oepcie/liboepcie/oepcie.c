#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define open _open
#define read _read
#define write _write
#define close _close
#define lseek _lseek
#else
#include <unistd.h>
#define _O_BINARY 0
#endif

#include "oepcie.h"

// Static fixed width types
typedef uint32_t oe_reg_val_t;

// Register size
#define OE_REGSZ   sizeof(oe_reg_val_t)

// Define if hardware produces byte-reversed types from compile command
#ifdef OE_BIGEND

// NB: Nested define allows compile command to define OE_BE
#define OE_BE

#define BSWAP_32(val)                                                          \
     ( (((uint32_t)(val)&0x000000ff) << 24)                                    \
     | (((uint32_t)(val)&0x0000ff00) << 8)                                     \
     | (((uint32_t)(val)&0x00ff0000) >> 8)                                     \
     | (((uint32_t)(val)&0xff000000) >> 24))

#endif

// Consistent overhead bytestuffing buffer size
#define OE_COBSBUFFERSIZE 255

// Bytes per read() syscall on the data input stream
// NB: This defines a minimum delay for real-time processing
#define OE_READSIZE 1024

typedef struct stream_fid {
    char *path;
    int fid;
} stream_fid_t;

// Context implementation
typedef struct oe_ctx_impl {

    // Communication channels
    stream_fid_t config;
    stream_fid_t read;
    stream_fid_t write;
    stream_fid_t signal;

    // Devices
    oe_size_t num_dev;
    oe_device_t* dev_map;

    // Maximum frame sizes (bytes)
    oe_size_t max_read_frame_size;
    oe_size_t max_write_frame_size;

    // Data buffer
    uint8_t *buffer;
    uint8_t *buff_read_pos;
    uint8_t *buff_end_pos;

    // Acqusition state
    enum {
        CTXNULL = 0,
        UNINITIALIZED,
        IDLE,
        RUNNING
    } run_state;

} oe_ctx_impl_t;

// Signal flags
typedef enum oe_signal {
    NULLSIG             = (1u << 0),
    CONFIGWACK          = (1u << 1), // Configuration write-acknowledgement
    CONFIGWNACK         = (1u << 2), // Configuration no-write-acknowledgement
    CONFIGRACK          = (1u << 3), // Configuration read-acknowledgement
    CONFIGRNACK         = (1u << 4), // Configuration no-read-acknowledgement
    DEVICEMAPACK        = (1u << 5), // Device map start acnknowledgement
    DEVICEINST          = (1u << 6), // Deivce map instance
} oe_signal_t;

// Configuration file offsets
typedef enum oe_conf_reg_off {

    // Register R/W interface
    CONFDEVIDOFFSET     = 0,   // Configuration device id register byte offset
    CONFADDROFFSET      = 4,   // Configuration register address register byte offset
    CONFVALUEOFFSET     = 8,   // Configuration register value register byte offset
    CONFRWOFFSET        = 12,  // Configuration register read/write register byte offset
    CONFTRIGOFFSET      = 16,  // Configuration read/write trigger register byte offset

    // Global configuration
    CONFRUNNINGOFFSET   = 20,  // Configuration run hardware register byte offset
    CONFRESETOFFSET     = 24,  // Configuration reset hardware register byte offset
    CONFSYSCLKHZOFFSET  = 28,  // Configuration base clock frequency register byte offset
    CONFFRAMEHZOFFSET   = 32,  // Configuration frame clock frequency register byte offset
} oe_conf_off_t;

// Static helpers
static inline int _oe_read(int data_fd, void* data, size_t size);
static inline int _oe_write(int data_fd, char* data, size_t size);
static inline int _oe_read_signal_packet(int signal_fd, uint8_t *buffer);
static int _oe_read_signal_data(int signal_fd, oe_signal_t *type, void *data, size_t size);
static int _oe_pump_signal_type(int signal_fd, int flags, oe_signal_t *type);
static int _oe_pump_signal_data(int signal_fd, int flags, oe_signal_t *type, void *data, size_t size);
static int _oe_cobs_unstuff(uint8_t *dst, const uint8_t *src, size_t size);
static int _oe_write_config(int config_fd, oe_conf_off_t write_offset, oe_reg_val_t value);
static int _oe_read_config(int config_fd, oe_conf_off_t read_offset, oe_reg_val_t *value);
static int _oe_read_buffer(oe_ctx ctx, void *data, size_t size);
//static inline int _oe_raw_size(oe_raw_t type);
#ifdef OE_BE
//static int _oe_array_bswap(void *data, oe_raw_t type, size_t size);
static int _device_map_byte_swap(oe_ctx ctx);
#endif

oe_ctx oe_create_ctx()
{
    oe_ctx ctx = calloc(1, sizeof(struct oe_ctx_impl));

    if (ctx == NULL) {
        errno = EAGAIN;
        return NULL;
    }

    // NB: Setting pointers to NULL Enables downstream use of realloc()
    ctx->config.path = NULL;
    ctx->read.path = NULL;
    ctx->write.path = NULL;
    ctx->signal.path = NULL;
    ctx->num_dev = 0;
    ctx->dev_map = NULL;
    ctx->run_state = UNINITIALIZED;

    return ctx;
}

int oe_init_ctx(oe_ctx ctx)
{
	assert(ctx != NULL && "Context is NULL.");
	assert(ctx->run_state == UNINITIALIZED && "Context is in invalid state.");

	if (ctx->run_state != UNINITIALIZED)
		return OE_EINVALSTATE;

    // Open the device files
	ctx->config.fid = open(ctx->config.path, O_RDWR | _O_BINARY);
	if (ctx->config.fid == -1) {
		fprintf(stderr, "%s: %s\n", strerror(errno), ctx->config.path);
		return OE_EPATHINVALID;
	}

	ctx->read.fid = open(ctx->read.path, O_RDONLY | _O_BINARY);
	if (ctx->read.fid == -1) {
		fprintf(stderr, "%s: %s\n", strerror(errno), ctx->read.path);
		return OE_EPATHINVALID;
	}

	ctx->write.fid = open(ctx->write.path, O_WRONLY | _O_BINARY);
	if (ctx->write.fid == -1) {
		fprintf(stderr, "%s: %s\n", strerror(errno), ctx->read.path);
		return OE_EPATHINVALID;
	}

	ctx->signal.fid = open(ctx->signal.path, O_RDONLY | _O_BINARY);
	if (ctx->signal.fid == -1) {
		fprintf(stderr, "%s: %s\n", strerror(errno), ctx->signal.path);
		return OE_EPATHINVALID;
	}

    // Reset the run and reset states of the hardware. This will push a frame
    // header onto the signal stream
    int rc = _oe_write_config(ctx->config.fid, CONFRUNNINGOFFSET, 0);
    if (rc) return rc;

    rc = _oe_write_config(ctx->config.fid, CONFRESETOFFSET, 1);
    if (rc) return rc;

    // Get number of devices
    oe_signal_t sig_type = NULLSIG;
    rc = _oe_pump_signal_data(
        ctx->signal.fid, DEVICEMAPACK, &sig_type, &(ctx->num_dev), sizeof(ctx->num_dev));
#ifdef OE_BE
    ctx->num_dev = BSWAP_32(ctx->num_dev);
#endif
    if (rc) return rc;

    // Make space for the device map
    oe_device_t *new_map
        = realloc(ctx->dev_map, ctx->num_dev * sizeof(oe_device_t));
    if (new_map)
        ctx->dev_map = new_map;
    else
        return OE_EBADALLOC;

    oe_size_t i;
    ctx->max_read_frame_size = 0;
    ctx->max_write_frame_size = 0;
    for (i = 0; i < ctx->num_dev; i++) {

        sig_type = NULLSIG;
        uint8_t buffer[OE_COBSBUFFERSIZE];
        rc = _oe_read_signal_data(ctx->signal.fid, &sig_type, buffer, OE_COBSBUFFERSIZE);
        if (rc) return rc;

        // We should see num_dev device instances appear on the signal stream
        if (sig_type != DEVICEINST)
            return OE_EBADDEVMAP;

        // Append the device onto the map
        memcpy(ctx->dev_map + i, buffer, sizeof(oe_device_t));
        ctx->max_read_frame_size += ctx->dev_map->read_size;
        ctx->max_write_frame_size += ctx->dev_map->write_size;
    }

#ifdef OE_BE
    _device_map_byte_swap(ctx);
#endif

    // Initialize buffer
    ctx->buff_read_pos = NULL;
    ctx->buff_end_pos = NULL;

    // We are now initialized and idle
    ctx->run_state = IDLE;

    return 0;
}

int oe_destroy_ctx(oe_ctx ctx)
{
    assert(ctx != NULL && "Context is NULL");

    if (ctx->run_state >= IDLE) {

        if (close(ctx->config.fid) == -1) goto oe_close_ctx_fail;
        if (close(ctx->read.fid) == -1) goto oe_close_ctx_fail;
        if (close(ctx->signal.fid) == -1) goto oe_close_ctx_fail;
    }

    free(ctx->config.path);
    free(ctx->read.path);
    free(ctx->write.path);
    free(ctx->signal.path);
    free(ctx->dev_map);
    free(ctx);

    return 0;

oe_close_ctx_fail:
    return OE_ECLOSEFAIL;
}

int oe_get_opt(const oe_ctx ctx, int option, void *value, size_t *option_len)
{
    assert(ctx != NULL && "Context is NULL");
    assert(ctx->run_state > UNINITIALIZED && "Context state must INITIALIZED.");
    if (ctx->run_state < IDLE)
        return OE_EINVALSTATE;

    switch (option) {

        case OE_CONFIGSTREAMPATH: {
            if (*option_len < (strlen(ctx->config.path) + 1))
                return OE_EBUFFERSIZE;

            size_t n = strlen(ctx->config.path) + 1;
            memcpy(value, ctx->config.path, n);
            *option_len = n;
            break;
        }
        case OE_READSTREAMPATH: {
            if (*option_len < (strlen(ctx->read.path) + 1))
                return OE_EBUFFERSIZE;

            size_t n = strlen(ctx->read.path) + 1;
            memcpy(value, ctx->read.path, n);
            *option_len = n;
            break;
        }
        case OE_WRITESTREAMPATH: {
            if (*option_len < (strlen(ctx->write.path) + 1))
                return OE_EBUFFERSIZE;

            size_t n = strlen(ctx->write.path) + 1;
            memcpy(value, ctx->write.path, n);
            *option_len = n;
            break;
        }
        case OE_SIGNALSTREAMPATH: {
            if (*option_len < (strlen(ctx->signal.path) + 1))
                return OE_EBUFFERSIZE;

            size_t n = strlen(ctx->signal.path) + 1;
            memcpy(value, ctx->signal.path, n);
            *option_len = n;
            break;
        }
        case OE_DEVICEMAP: {
            size_t required_bytes = sizeof(oe_device_t) * ctx->num_dev;
            if (*option_len < required_bytes)
                return OE_EBUFFERSIZE;

            memcpy(value, ctx->dev_map, required_bytes);
            *option_len = required_bytes;
            break;
        }
        case OE_NUMDEVICES: {
            size_t required_bytes = sizeof(oe_size_t);
            if (*option_len < required_bytes)
                return OE_EBUFFERSIZE;

            *(oe_size_t *)value = ctx->num_dev;
            *option_len = required_bytes;
            break;
        }
        case OE_MAXREADFRAMESIZE: {
            size_t required_bytes = sizeof(oe_size_t);
            if (*option_len < required_bytes)
                return OE_EBUFFERSIZE;

            *(oe_size_t *)value = ctx->max_read_frame_size;
            *option_len = required_bytes;
            break;
        }
        case OE_MAXWRITEFRAMESIZE: {
            size_t required_bytes = sizeof(oe_size_t);
            if (*option_len < required_bytes)
                return OE_EBUFFERSIZE;

            *(oe_size_t *)value = ctx->max_write_frame_size;
            *option_len = required_bytes;
            break;
        }
        case OE_RUNNING: {
            if (*option_len != OE_REGSZ)
                return OE_EBUFFERSIZE;

            int rc = _oe_read_config(ctx->config.fid, CONFRUNNINGOFFSET, value);
            if (rc)
                return rc;

            *option_len = OE_REGSZ;
            break;
        }
        case OE_RESET: {
            if (*option_len != OE_REGSZ)
                return OE_EBUFFERSIZE;

            int rc = _oe_read_config(ctx->config.fid, CONFRESETOFFSET, value);
            if (rc)
                return rc;

            *option_len = OE_REGSZ;
            break;
        }
        case OE_SYSCLKHZ: {
            if (*option_len != OE_REGSZ)
                return OE_EBUFFERSIZE;

            int rc
                = _oe_read_config(ctx->config.fid, CONFSYSCLKHZOFFSET, value);
            if (rc)
                return rc;

            *option_len = OE_REGSZ;
            break;
        }

        default:
            return OE_EINVALOPT;
    }

    return 0;
}

int oe_set_opt(oe_ctx ctx, int option, const void *value, size_t option_len)
{
    assert(ctx != NULL && "Context is NULL");

    switch (option) {

        case OE_CONFIGSTREAMPATH: {
            assert(ctx->run_state == UNINITIALIZED && "Context state must be UNINITIALIZED.");
            if (ctx->run_state != UNINITIALIZED)
                return OE_EINVALSTATE;
            ctx->config.path = realloc(ctx->config.path, option_len);
            memcpy(ctx->config.path, value, option_len);
            break;
        }
        case OE_READSTREAMPATH: {
            assert(ctx->run_state == UNINITIALIZED && "Context state must be UNINITIALIZED.");
            if (ctx->run_state != UNINITIALIZED)
                return OE_EINVALSTATE;
            ctx->read.path = realloc(ctx->read.path, option_len);
            memcpy(ctx->read.path, value, option_len);
            break;
        }
        case OE_WRITESTREAMPATH: {
            assert(ctx->run_state == UNINITIALIZED && "Context state must be UNINITIALIZED.");
            if (ctx->run_state != UNINITIALIZED)
                return OE_EINVALSTATE;
            ctx->write.path = realloc(ctx->write.path, option_len);
            memcpy(ctx->write.path, value, option_len);
            break;
        }
        case OE_SIGNALSTREAMPATH: {
            assert(ctx->run_state == UNINITIALIZED && "Context state must be UNINITIALIZED.");
            if (ctx->run_state != UNINITIALIZED)
                return OE_EINVALSTATE;
            ctx->signal.path = realloc(ctx->signal.path, option_len);
            memcpy(ctx->signal.path, value, option_len);
            break;
        }
        case OE_RUNNING: {
            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return OE_EINVALSTATE;

            if (option_len != OE_REGSZ)
                return OE_EBUFFERSIZE;

            int rc = _oe_write_config(
                ctx->config.fid, CONFRUNNINGOFFSET, *(oe_reg_val_t*)value);
            if (rc) return rc;

            if (*(oe_reg_val_t *)value != 0)
                ctx->run_state = RUNNING;
            else
                ctx->run_state = IDLE;
            break;
        }
        case OE_RESET: {
            assert(ctx->run_state > UNINITIALIZED && "Context state must be IDLE or RUNNING.");
            if (ctx->run_state < IDLE)
                return OE_EINVALSTATE;

            if (option_len != OE_REGSZ)
                return OE_EBUFFERSIZE;

            int rc = _oe_write_config(
                ctx->config.fid, CONFRESETOFFSET, *(oe_reg_val_t*)value);
            if (rc) return rc;

            if (*(oe_reg_val_t *)value != 0)
                ctx->run_state = IDLE;

            break;
        }
        case OE_SYSCLKHZ: {
            return OE_EREADONLY;
        }
        default:
            return OE_EINVALOPT;
    }

    return 0;
}

int oe_write_reg(const oe_ctx ctx,
                 size_t dev_idx,
                 oe_reg_addr_t addr,
                 oe_reg_val_t value)
{
    assert(ctx != NULL && "Context is NULL");
    assert(ctx->run_state > UNINITIALIZED && "Context must be INITIALIZED.");

    // Checks that the device index is valid
    if (dev_idx >= ctx->num_dev && dev_idx)
        return OE_EDEVIDX;

    if (lseek(ctx->config.fid, CONFTRIGOFFSET, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    // Make sure we are not already in config triggered state
    uint32_t trig = 0x00;
    if (read(ctx->config.fid, &trig, sizeof(uint32_t)) == 0)
        return OE_EREADFAILURE;

    if (trig != 0)
        return OE_ERETRIG;

    // Set config registers and trigger a write
    if (lseek(ctx->config.fid, CONFDEVIDOFFSET, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    if (write(ctx->config.fid, &dev_idx, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    if (write(ctx->config.fid, &addr, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    if (write(ctx->config.fid, &value, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    uint32_t rw = 0x01;
    if (write(ctx->config.fid, &rw, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    trig = 0x01;
    if (write(ctx->config.fid, &trig, sizeof(uint32_t)) <= 0)
        return OE_EWRITEFAILURE;

    oe_signal_t type;
    int rc = _oe_pump_signal_type(ctx->signal.fid, CONFIGWACK | CONFIGWNACK, &type);
    if (rc) return rc;

    if (type == CONFIGWNACK)
        return OE_EREADONLY;

    return 0;
}

int oe_read_reg(const oe_ctx ctx,
                size_t dev_idx,
                oe_reg_addr_t addr,
                oe_reg_val_t *value)
{
    assert(ctx != NULL && "Context is NULL");
    assert(ctx->run_state > UNINITIALIZED && "Context must be INITIALIZED.");

    // Checks that the device index is valid
    if (dev_idx >= ctx->num_dev && dev_idx)
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

    if (write(ctx->config.fid, &dev_idx, sizeof(uint32_t)) <= 0)
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

    oe_signal_t type;
    int rc = _oe_pump_signal_data(
        ctx->signal.fid, CONFIGRACK | CONFIGRNACK, &type, value, sizeof(*value));
    if (rc) return rc;

    if (type == CONFIGRNACK)
        return OE_EREADONLY;

    return 0;
}

int oe_read_frame(const oe_ctx ctx, oe_frame_t **frame)
{
    assert(ctx != NULL && "Context is NULL");

    // NB: We don't need run_state == RUNNING because this could be changed in
    // a different thread
    assert(ctx->run_state >= IDLE && "Context is not acquiring.");

    // Get the header and figure out how many devies are in the frame
    uint8_t header[OE_RFRAMEHEADERSZ];
    int rc = _oe_read_buffer(ctx, header, OE_RFRAMEHEADERSZ);
    if (rc != 0) return rc;

    // Allocate frame
    *frame = malloc(sizeof(oe_frame_t));
    oe_frame_t *f_ptr = *frame;

    // Copy frame header members
    memcpy(&f_ptr->clock, header, sizeof(uint64_t));
    memcpy(&f_ptr->num_dev,header + OE_RFRAMENDEVOFF, sizeof(uint16_t));
    memcpy(&f_ptr->corrupt, header + OE_RFRAMENERROFF, sizeof(uint8_t));

    // Allocate space for device info
    f_ptr->dev_idxs_sz = f_ptr->num_dev * sizeof(oe_size_t);
    f_ptr->dev_offs_sz = f_ptr->num_dev * sizeof(oe_size_t);
    f_ptr->dev_idxs = malloc(f_ptr->dev_idxs_sz);
    f_ptr->dev_offs = malloc(f_ptr->dev_offs_sz);

    // Read device indices that are in this frame
    rc = _oe_read_buffer(ctx, f_ptr->dev_idxs, f_ptr->dev_idxs_sz);
    if (rc != 0) return rc;
    //assert((size_t)rc == f_ptr->dev_idxs_sz && "Did not read full dev idxs buffer.");

    // Find data read size
    uint16_t i;
    int rsize = 0;
    for (i = 0; i < f_ptr->num_dev; i++) {
        *(f_ptr->dev_offs + i) = rsize;
        rsize += ctx->dev_map[*(f_ptr->dev_idxs + i)].read_size;
    }

    // Read size + padding
    rsize += rsize % 4;

    // Now we know the frame data size, so we allocate
    f_ptr->data_sz = rsize;
    f_ptr->data = malloc(rsize);

    // Read data
    rc = _oe_read_buffer(ctx, f_ptr->data, rsize);
    if (rc != 0) return rc;
    //assert(rc == rsize && "Did not read full data buffer.");

    return 0;
}

int oe_write_frame(const oe_ctx ctx, oe_frame_t *frame)
{
    int num_bytes = OE_WFRAMEHEADERSZ + frame->dev_idxs_sz + frame->data_sz;

    // Create serialized write frame
    char *write_frame = malloc(num_bytes);
    // TODO: Header
    memcpy(write_frame + OE_WFRAMEHEADERSZ, frame->dev_idxs, frame->dev_idxs_sz);
    memcpy(write_frame + OE_WFRAMEHEADERSZ + frame->dev_idxs_sz, frame->data, frame->data_sz);

    int rc = _oe_write(ctx->write.fid, write_frame, num_bytes);

    free(write_frame);

    if (rc != num_bytes) return rc;
    return 0;
}

void oe_destroy_frame(oe_frame_t *frame)
{
    if (frame != NULL) {

        // Free held resources
        if (frame->dev_idxs != NULL)
            free(frame->dev_idxs);
        if (frame->dev_offs != NULL)
            free(frame->dev_offs);
        if (frame->data != NULL)
            free(frame->data);

        // Free the container
        free(frame);
    }
}

void oe_version(int *major, int *minor, int *patch)
{
    *major = OE_VERSION_MAJOR;
    *minor = OE_VERSION_MINOR;
    *patch = OE_VERSION_PATCH;
}

const char *oe_error_str(int err)
{
    assert(err > OE_MINERRORNUM && "Invalid error number.");
    assert(err <= 0 && "Invalid error number.");

    switch (err) {
        case OE_ESUCCESS: {
            return "Success";
        }
        case OE_EPATHINVALID: {
            return "Invalid stream path";
        }
        case OE_EREINITCTX: {
            return "Double context initialization attempt";
        }
        case OE_EDEVID: {
            return "Invalid device ID";
        }
        case OE_EREADFAILURE: {
            return "Failure to read from a stream or register";
        }
        case OE_EWRITEFAILURE: {
            return "Failure to write to a stream or register";
        }
        case OE_ENULLCTX: {
            return "Null context";
        }
        case OE_ESEEKFAILURE: {
            return "Failure to seek on stream";
        }
        case OE_EINVALSTATE: {
            return "Invalid operation for the current context run state";
        }
        case OE_EDEVIDX: {
            return "Invalid device index";
        }
        case OE_EINVALOPT: {
            return "Invalid context option";
        }
        case OE_EINVALARG: {
            return "Invalid function arguments";
        }
        case OE_ECANTSETOPT: {
            return "Option cannot be set in current context state";
        }
        case OE_ECOBSPACK: {
            return "Invalid COBS packet";
        }
        case OE_ERETRIG: {
            return "Attempt to trigger an already triggered operation";
        }
        case OE_EBUFFERSIZE: {
            return "Supplied buffer is too small";
        }
        case OE_EBADDEVMAP: {
            return "Badly formatted device map supplied by firmware";
        }
        case OE_EBADALLOC: {
            return "Bad dynamic memory allocation";
        }
        case OE_ECLOSEFAIL: {
            return "File descriptor close failure (check errno)";
        }
        case OE_EDATATYPE: {
            return "Invalid underlying data types";
        }
        case OE_EREADONLY: {
            return "Attempted write to read only object (register, context "
                   "option, etc)";
        }
        case OE_ERUNSTATESYNC: {
            return "Software and hardware run state out of sync";
        }
        case OE_EINVALRAWTYPE: {
            return "Invalid raw data type";
        }
        default:
            return "Unknown error";
    }
}

static inline int _oe_read(int data_fd, void *data, size_t size)
{
    size_t received = 0;

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

static inline int _oe_write(int data_fd, char *data, size_t size)
{
    int written = 0;

    while (1) {

        written = write(data_fd, data, size);

        if ((written < 0) && (errno == EINTR))
            continue;

        if (written <= 0)
            return OE_EWRITEFAILURE;
    }

    return written;
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

static int _oe_read_signal_data(int signal_fd, oe_signal_t *type, void *data, size_t size)
{
    if (type == NULL)
        return OE_EINVALARG;

    uint8_t buffer[255] = {0};

    int pack_size = _oe_read_signal_packet(signal_fd, buffer);
    if (pack_size < 0) return pack_size;

    // Unstuff the packet (last byte is the 0, so we decrement
    int rc = _oe_cobs_unstuff(buffer, buffer, pack_size);
    if (rc < 0) return rc;

    if ((int)size < (--pack_size - 4))
        return OE_EBUFFERSIZE;

    // Get the type, which occupies first 4 bytes of buffer
    memcpy(type, buffer, sizeof(oe_signal_t));
#ifdef OE_BE
    *type = BSWAP_32(*type);
#endif

    // pack_size still has overhead byte and header, so we remove those
    size_t data_size = pack_size - 1 - sizeof(oe_signal_t);
    if (size < data_size)
        return OE_EBUFFERSIZE;

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
        memcpy(&packet_type, buffer, sizeof(oe_signal_t));
#ifdef OE_BE
        packet_type = BSWAP_32(packet_type);
#endif

    } while (!(packet_type & flags));

    *type = packet_type;

    return 0;
}

static int _oe_pump_signal_data(
    int signal_fd, int flags, oe_signal_t *type, void *data, size_t size)
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
        memcpy(&packet_type, buffer, sizeof(oe_signal_t));
#ifdef OE_BE
        packet_type = BSWAP_32(packet_type);
#endif

    } while (!(packet_type & flags));

    *type = packet_type;

    // pack_size still has overhead byte and header, so we remove those
    size_t data_size = pack_size - 1 - sizeof(oe_signal_t);
    if (size < data_size)
        return OE_EBUFFERSIZE;

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

static int _oe_write_config(int config_fd,
                            oe_conf_off_t write_offset,
                            oe_reg_val_t value)
{
    if (lseek(config_fd, write_offset, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    if (write(config_fd, &value, OE_REGSZ) != OE_REGSZ)
        return OE_EWRITEFAILURE;

    return 0;
}

static int _oe_read_config(int config_fd,
                           oe_conf_off_t read_offset,
                           oe_reg_val_t *value)
{
    if (lseek(config_fd, read_offset, SEEK_SET) < 0)
        return OE_ESEEKFAILURE;

    if (read(config_fd, value, OE_REGSZ) != OE_REGSZ)
        return OE_EREADFAILURE;

    return 0;
}

static int _oe_read_buffer(oe_ctx ctx, void *data, size_t size)
{
    // Check if we have less than size remaining
    size_t remaining = ctx->buff_end_pos - ctx->buff_read_pos;

    // Buffer reup?
    if (remaining < size) {

        // Allocate space in buffer
        uint8_t *old_buffer = ctx->buffer;
        ctx->buffer = malloc(remaining + OE_READSIZE);

        // Transfer remaining data to new buffer
        if (old_buffer != NULL) {
            memcpy(ctx->buffer, ctx->buff_read_pos, remaining);
            free(old_buffer);
        }

        // Reset read and end positions
        ctx->buff_read_pos = ctx->buffer;
        ctx->buff_end_pos = ctx->buffer + remaining + OE_READSIZE;

        // Fill the buffer
        int rc = _oe_read(ctx->read.fid, ctx->buffer + remaining, OE_READSIZE);
        if (rc != OE_READSIZE) return rc;
    }

    // Read from buffer
    memcpy(data, ctx->buff_read_pos, size);
    ctx->buff_read_pos += size;

    return 0;
}

#ifdef OE_BE
static int _device_map_byte_swap(oe_ctx ctx)
{
    size_t i;
    for (i = 0; i < ctx->num_dev; i++) {
        ctx->dev_map[i].id = BSWAP_32(ctx->dev_map[i].id);
        ctx->dev_map[i].read_size = BSWAP_32(ctx->dev_map[i].read_size);
        ctx->dev_map[i].num_reads = BSWAP_32(ctx->dev_map[i].num_reads);
        ctx->dev_map[i].write_size = BSWAP_32(ctx->dev_map[i].write_size);
        ctx->dev_map[i].num_writes = BSWAP_32(ctx->dev_map[i].num_writes);
    }

    return 0;
}

#endif
