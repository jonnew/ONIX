#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "onidriver_xillybus.h"
#include "../../onidriver.h"

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

#define UNUSED(x) (void)(x)

// to save some repetition
#define CTX_CAST const oni_xillybus_ctx ctx = (oni_xillybus_ctx)driver_ctx

struct stream_fid {
    char *path;
    int fid;
};

struct oni_xillybus_ctx_impl {

    // Communication channels
    struct stream_fid config;
    struct stream_fid read;
    struct stream_fid write;
    struct stream_fid signal;

    enum { CLOSED, OPEN } file_state;
};

typedef struct oni_xillybus_ctx_impl* oni_xillybus_ctx;

// Configuration file offsets
typedef enum oni_conf_reg_off {
    // Register R/W interface
    CONFDEVIDXOFFSET = 0,   // Configuration device index register byte offset
    CONFADDROFFSET = 4,   // Configuration register address register byte offset
    CONFVALUEOFFSET = 8,   // Configuration register value register byte offset
    CONFRWOFFSET = 12,  // Configuration register read/write register byte offset
    CONFTRIGOFFSET = 16,  // Configuration read/write trigger register byte offset

    // Global configuration
    CONFRUNNINGOFFSET = 20,  // Configuration run hardware register byte offset
    CONFRESETOFFSET = 24,  // Configuration reset hardware register byte offset
    CONFSYSCLKHZOFFSET = 28,  // Configuration base clock frequency register byte offset
    CONFACQCLKHZOFFSET = 32, // Configuration frame counter clock frequency register byte offset
    CONFRESETACQCOUNTER = 36, // Configuration frame counter clock reset register byte offset
    CONFHWADDRESS = 40 // Configuration hardware address register byte offset
} oni_conf_off_t;

static inline oni_conf_off_t _oni_register_offset(oni_config_t reg);

oni_driver_ctx oni_driver_create_ctx()
{
    oni_xillybus_ctx ctx;
    ctx = calloc(1, sizeof(struct oni_xillybus_ctx_impl));
    if (ctx == NULL)
        return NULL;

    // Set default paths
    ctx->config.path = ONI_XILLYBUS_DEFAULTCONFIGPATH;
    ctx->read.path = ONI_XILLYBUS_DEFAULTREADPATH;
    ctx->write.path = ONI_XILLYBUS_DEFAULTWRITEPATH;
    ctx->signal.path = ONI_XILLYBUS_DEFAULTSIGNALPATH;
    ctx->file_state = CLOSED;

    return ctx;
}

int oni_driver_init(oni_driver_ctx driver_ctx, int host_idx)
{
    UNUSED(host_idx);

    CTX_CAST;
    // Open the device files
    ctx->config.fid = open(ctx->config.path, O_RDWR | _O_BINARY);
    if (ctx->config.fid == -1) {
        //fprintf(stderr, "%s: %s\n", strerror(errno), ctx->config.path);
        return ONI_EPATHINVALID;
    }

    ctx->signal.fid = open(ctx->signal.path, O_RDONLY | _O_BINARY);
    if (ctx->signal.fid == -1) {
        //fprintf(stderr, "%s: %s\n", strerror(errno), ctx->signal.path);
        close(ctx->config.fid);
        return ONI_EPATHINVALID;
    }

    ctx->read.fid = open(ctx->read.path, O_RDONLY | _O_BINARY);
    if (ctx->read.fid == -1) {
        //fprintf(stderr, "%s: %s\n", strerror(errno), ctx->read.path);
        close(ctx->config.fid);
        close(ctx->signal.fid);
        return ONI_EPATHINVALID;
    }

    ctx->write.fid = open(ctx->write.path, O_WRONLY | _O_BINARY);
    if (ctx->write.fid == -1) {
        //fprintf(stderr, "%s: %s\n", strerror(errno), ctx->write.path);
        close(ctx->config.fid);
        close(ctx->signal.fid);
        close(ctx->read.fid);
        return ONI_EPATHINVALID;
    }
    ctx->file_state = OPEN;

    return ONI_ESUCCESS;
}

int oni_driver_destroy_ctx(oni_driver_ctx driver_ctx)
{
    CTX_CAST;

    assert(ctx != NULL && "Driver context is NULL");

    if (ctx->file_state >= OPEN) {

        if (close(ctx->config.fid) == -1) goto oni_close_ctx_fail;
        if (close(ctx->read.fid) == -1) goto oni_close_ctx_fail;
        if (close(ctx->write.fid) == -1) goto oni_close_ctx_fail;
        if (close(ctx->signal.fid) == -1) goto oni_close_ctx_fail;
    }

    free(ctx->config.path);
    free(ctx->read.path);
    free(ctx->write.path);
    free(ctx->signal.path);
    free(ctx);

    return ONI_ESUCCESS;

oni_close_ctx_fail:
    return ONI_ECLOSEFAIL;
}

int oni_driver_read_stream(oni_driver_ctx driver_ctx,
                           oni_read_stream_t stream,
                           void *data,
                           size_t size)
{
    CTX_CAST;
    size_t received = 0;

    int data_fd;
    switch (stream) {
        case ONI_READ_STREAM_DATA:
            data_fd = ctx->read.fid;
            break;
        case ONI_READ_STREAM_SIGNAL:
            data_fd = ctx->signal.fid;
            break;
        default:
            return ONI_EPATHINVALID;
    }

    while (received < size) {

        int rc = read(data_fd, (char *)data + received, size - received);

        if ((rc < 0) && (errno == EINTR))
            continue;

        if (rc <= 0)
            return ONI_EREADFAILURE;

        received += rc;
    }

    return received;
}

int oni_driver_write_stream(oni_driver_ctx driver_ctx,
                            oni_write_stream_t stream,
                            const char *data,
                            size_t size)
{
    CTX_CAST;
    size_t written = 0;

    int data_fd;
    switch (stream)
    {
    case ONI_WRITE_STREAM_DATA:
        data_fd = ctx->write.fid;
        break;
    default:
        return ONI_EPATHINVALID;
    }

    while (written < size) {

        int rc = write(data_fd, data + written, size - written);

        if ((rc < 0) && (errno == EINTR))
            continue;

        if (rc <= 0)
            return ONI_EWRITEFAILURE;

        written += rc;
    }

    return written;
}

int oni_driver_write_config(oni_driver_ctx driver_ctx,
                            oni_config_t reg,
                            oni_reg_val_t value)
{
    CTX_CAST;
    oni_conf_off_t write_offset = _oni_register_offset(reg);
    int config_fd = ctx->config.fid;

    if (lseek(config_fd, write_offset, SEEK_SET) < 0)
        return ONI_ESEEKFAILURE;

    if (write(config_fd, &value, ONI_REGSZ) != ONI_REGSZ)
        return ONI_EWRITEFAILURE;

    return ONI_ESUCCESS;
}

int oni_driver_read_config(oni_driver_ctx driver_ctx, oni_config_t reg, oni_reg_val_t* value)
{
    CTX_CAST;
    oni_conf_off_t read_offset = _oni_register_offset(reg);
    int config_fd = ctx->config.fid;

    if (lseek(config_fd, read_offset, SEEK_SET) < 0)
        return ONI_ESEEKFAILURE;

    if (read(config_fd, value, ONI_REGSZ) != ONI_REGSZ)
        return ONI_EREADFAILURE;

    return ONI_ESUCCESS;
}

//Right now we do not do anything for the common options
int oni_driver_set_opt_callback(oni_driver_ctx driver_ctx,
                                int oni_option,
                                const void *value,
                                size_t option_len)
{
    UNUSED(driver_ctx);
    UNUSED(oni_option);
    UNUSED(value);
    UNUSED(option_len);
    return ONI_ESUCCESS;
}

int oni_driver_set_opt(oni_driver_ctx driver_ctx,
                       int driver_option,
                       const void *value,
                       size_t option_len)
{
    CTX_CAST;
    switch (driver_option) {
        case ONI_XILLYBUS_CONFIGSTREAMPATH: {
            assert(ctx->file_state == CLOSED && "Context state must be UNINITIALIZED.");
            if (ctx->file_state != CLOSED)
                return ONI_EINVALSTATE;
            ctx->config.path = realloc(ctx->config.path, option_len);
            memcpy(ctx->config.path, value, option_len);
            break;
        }
        case ONI_XILLYBUS_READSTREAMPATH: {
            assert(ctx->file_state == CLOSED && "Context state must be UNINITIALIZED.");
            if (ctx->file_state != CLOSED)
                return ONI_EINVALSTATE;
            ctx->read.path = realloc(ctx->read.path, option_len);
            memcpy(ctx->read.path, value, option_len);
            break;
        }
        case ONI_XILLYBUS_WRITESTREAMPATH: {
            assert(ctx->file_state == CLOSED && "Context state must be UNINITIALIZED.");
            if (ctx->file_state != CLOSED)
                return ONI_EINVALSTATE;
            ctx->write.path = realloc(ctx->write.path, option_len);
            memcpy(ctx->write.path, value, option_len);
            break;
        }
        case ONI_XILLYBUS_SIGNALSTREAMPATH: {
            assert(ctx->file_state == CLOSED && "Context state must be UNINITIALIZED.");
            if (ctx->file_state != CLOSED)
                return ONI_EINVALSTATE;
            ctx->signal.path = realloc(ctx->signal.path, option_len);
            memcpy(ctx->signal.path, value, option_len);
            break;
        }
        default:
            return ONI_EINVALOPT;
    }
    return ONI_ESUCCESS;
}

int oni_driver_get_opt(oni_driver_ctx driver_ctx,
                       int driver_option,
                       void *value,
                       size_t *option_len)
{
    CTX_CAST;
    switch (driver_option) {
        case ONI_XILLYBUS_CONFIGSTREAMPATH: {
            if (*option_len < (strlen(ctx->config.path) + 1))
                return ONI_EBUFFERSIZE;

            size_t n = strlen(ctx->config.path) + 1;
            memcpy(value, ctx->config.path, n);
            *option_len = n;
            break;
        }
        case ONI_XILLYBUS_READSTREAMPATH: {
            if (*option_len < (strlen(ctx->read.path) + 1))
                return ONI_EBUFFERSIZE;

            size_t n = strlen(ctx->read.path) + 1;
            memcpy(value, ctx->read.path, n);
            *option_len = n;
            break;
        }
        case ONI_XILLYBUS_WRITESTREAMPATH: {
            if (*option_len < (strlen(ctx->write.path) + 1))
                return ONI_EBUFFERSIZE;

            size_t n = strlen(ctx->write.path) + 1;
            memcpy(value, ctx->write.path, n);
            *option_len = n;
            break;
        }
        case ONI_XILLYBUS_SIGNALSTREAMPATH: {
            if (*option_len < (strlen(ctx->signal.path) + 1))
                return ONI_EBUFFERSIZE;

            size_t n = strlen(ctx->signal.path) + 1;
            memcpy(value, ctx->signal.path, n);
            *option_len = n;
            break;
        }
        default:
            return ONI_EINVALOPT;
    }
    return ONI_ESUCCESS;
}

const char* oni_driver_str()
{
    return XILLYBUS_DRIVER_NAME;
}

static inline oni_conf_off_t _oni_register_offset(oni_config_t reg)
{
    switch (reg) {
        case ONI_CONFIG_DEV_IDX:
            return CONFDEVIDXOFFSET;
        case ONI_CONFIG_REG_ADDR:
            return CONFADDROFFSET;
        case ONI_CONFIG_REG_VALUE:
            return CONFVALUEOFFSET;
        case ONI_CONFIG_RW:
            return CONFRWOFFSET;
        case ONI_CONFIG_TRIG:
            return CONFTRIGOFFSET;
        case ONI_CONFIG_RUNNING:
            return CONFRUNNINGOFFSET;
        case ONI_CONFIG_RESET:
            return CONFRESETOFFSET;
        case ONI_CONFIG_SYSCLKHZ:
            return CONFSYSCLKHZOFFSET;
        case ONI_CONFIG_ACQCLKHZ:
            return CONFACQCLKHZOFFSET;
        case ONI_CONFIG_RESETACQCOUNTER:
            return CONFRESETACQCOUNTER;
        case ONI_CONFIG_HWADDRESS:
            return CONFHWADDRESS;
        default:
            return 0;
    }
}
