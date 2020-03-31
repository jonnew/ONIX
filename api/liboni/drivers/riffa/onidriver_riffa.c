#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../onidriver.h"
#include <riffa.h>

#define UNUSED(x) (void)(x)

// to save some repetition
#define CTX_CAST const oni_riffa_ctx ctx = (oni_riffa_ctx)driver_ctx

#define MIN(a,b) ((a<b) ? a : b)

struct oni_riffa_ctx_impl {
    oni_size_t block_size;
    fpga_t* fpga;
};

typedef struct oni_riffa_ctx_impl* oni_riffa_ctx;

// Configuration file offsets
typedef enum oni_conf_reg_off {
    // Register R/W interface
    CONFDEVIDXOFFSET = 0, // Configuration device index register byte offset
    CONFADDROFFSET = 1, // Configuration register address register byte offset
    CONFVALUEOFFSET = 2, // Configuration register value register byte offset
    CONFRWOFFSET = 3, // Configuration register read/write register byte offset
    CONFTRIGOFFSET = 4, // Configuration read/write trigger register byte offset

    // Global configuration
    CONFRUNNINGOFFSET = 5, // Configuration run hardware register byte offset
    CONFRESETOFFSET = 6, // Configuration reset hardware register byte offset
    CONFSYSCLKHZOFFSET = 7, // Configuration base clock frequency register byte offset
} oni_conf_off_t;

static inline oni_conf_off_t _oni_register_offset(oni_config_t reg);

enum riffa_channels {
    RIFFA_CONFIG = 0,
    RIFFA_WRITE = 1,
    RIFFA_READ = 2,
    RIFFA_SIGNAL = 3
};

static const oni_size_t write_block_size = 1024;

oni_driver_ctx oni_driver_create_ctx()
{
    oni_riffa_ctx ctx;
    ctx = calloc(1, sizeof(struct oni_riffa_ctx_impl));
    if (ctx == NULL)
        return NULL;

    ctx->block_size = 1024;
    ctx->fpga = NULL;

    return ctx;
}

int oni_driver_init(oni_driver_ctx driver_ctx, int host_idx)
{
    CTX_CAST;
    if (host_idx < 0) host_idx = 0;
    ctx->fpga = fpga_open(host_idx);
    if (ctx->fpga == NULL) return ONI_EINIT;

    //reset the whole system
    fpga_reset(ctx->fpga);
    return ONI_ESUCCESS;
}

int oni_driver_destroy_ctx(oni_driver_ctx driver_ctx)
{
    CTX_CAST;

    assert(ctx != NULL && "Driver context is NULL");

    if (ctx->fpga != NULL)
    {
        fpga_reset(ctx->fpga); //Let's keep the fpga turned off, just in case
        fpga_close(ctx->fpga);
    }
    free(ctx);

    return ONI_ESUCCESS;
}

int oni_driver_read_stream(oni_driver_ctx driver_ctx,
    oni_read_stream_t stream,
    void *data,
    size_t size)
{
    CTX_CAST;
    int rc;
    uint32_t words = size >> 2;

    if (stream == ONI_READ_STREAM_DATA)
    {
        if (words != ctx->block_size) return ONI_EINVALREADSIZE;
        rc = fpga_recv(ctx->fpga, RIFFA_READ, data, words, 0);
        if (rc < 1) return ONI_EREADFAILURE;
        return (rc << 2);
    }
    else if (stream == ONI_READ_STREAM_SIGNAL)
    {
        oni_size_t read = 0;
        uint32_t read_word;
        while (read < size)
        {
            rc = fpga_recv(ctx->fpga, RIFFA_SIGNAL, &read_word, 1, 0);
            if (rc < 1) return ONI_EREADFAILURE;
            ((uint8_t*)data)[read] = (uint8_t)(read_word & 0xFF);
            read++;
        }
        return read;
    }
    else return ONI_EPATHINVALID;
}

int oni_driver_write_stream(oni_driver_ctx driver_ctx,
    oni_write_stream_t stream,
    const char *data,
    size_t size)
{
    CTX_CAST;
    size_t remaining = size;
    size_t to_send, sent;
    uint32_t* ptr = (uint32_t*)data;

    if (stream != ONI_WRITE_STREAM_DATA) return ONI_EPATHINVALID;

    while (remaining > 0)
    {
        to_send = MIN(remaining, write_block_size);
        sent = fpga_send(ctx->fpga, RIFFA_WRITE, ptr, to_send, 0, 1, 0);
        if (sent != to_send) return ONI_EWRITEFAILURE;
        ptr += sent;
        remaining -= sent;
    }
    return size;
}

int oni_driver_write_config(oni_driver_ctx driver_ctx,
    oni_config_t reg,
    oni_reg_val_t value)
{
    CTX_CAST;
    uint32_t addr;
    int rc;

    //Prior to an ONI reset, reset the whole fpga to ensure that DMA transmission buffers are clear
    if (reg == ONI_CONFIG_RESET && value != 0)
        fpga_reset(ctx->fpga);

    oni_conf_off_t write_offset = _oni_register_offset(reg);

    addr = write_offset;
    rc = fpga_send(ctx->fpga, RIFFA_CONFIG, &value, 1, addr, 1, 0);
    if (rc < 1) return ONI_EWRITEFAILURE;

    return ONI_ESUCCESS;
}

int oni_driver_read_config(oni_driver_ctx driver_ctx, oni_config_t reg, oni_reg_val_t* value)
{
    CTX_CAST;
    uint32_t addr;
    int rc;
    oni_conf_off_t write_offset = _oni_register_offset(reg);

    addr = write_offset | (1 << 29);
    rc = fpga_send(ctx->fpga, RIFFA_CONFIG, value, 1, addr, 1, 0);
    if (rc < 1) return ONI_EWRITEFAILURE;
    rc = fpga_recv(ctx->fpga, RIFFA_CONFIG, value, 1, 0);
    if (rc < 1) return ONI_EREADFAILURE;

    return ONI_ESUCCESS;
}

int oni_driver_set_opt_callback(oni_driver_ctx driver_ctx,
    int oni_option,
    const void *value,
    size_t option_len)
{
    int rc;
    CTX_CAST;
    UNUSED(option_len);

    if (oni_option == ONI_OPT_RUNNING && *(uint32_t*)value == 0)
    {
        //When acquisition is disabled we need to empty the DMA buffers
        uint32_t* dummydata = malloc(ctx->block_size * sizeof(uint32_t));
        do {
            rc = fpga_recv(ctx->fpga, RIFFA_READ, dummydata, ctx->block_size, 200);
        } while (rc >= ctx->block_size && rc > 0); // Negative rc results from timeout.
        free(dummydata);

    }
    else if (oni_option == ONI_OPT_BLOCKREADSIZE)
    {
        ctx->block_size = ((*(oni_size_t*)value) >> 2);
        //configure block size in board
        rc = fpga_send(ctx->fpga, RIFFA_READ, &ctx->block_size, 1, 0, 1, 0);
        if (rc < 1) return ONI_EWRITEFAILURE;
    }
    return ONI_ESUCCESS;
}

//this driver does not have custom options
int oni_driver_set_opt(oni_driver_ctx driver_ctx,
    int driver_option,
    const void *value,
    size_t option_len)
{
    UNUSED(driver_ctx);
    UNUSED(driver_option);
    UNUSED(value);
    UNUSED(option_len);
    return ONI_EINVALOPT;
}

int oni_driver_get_opt(oni_driver_ctx driver_ctx,
    int driver_option,
    void *value,
    size_t *option_len)
{
    UNUSED(driver_ctx);
    UNUSED(driver_option);
    UNUSED(value);
    UNUSED(option_len);
    return ONI_EINVALOPT;
}

const char* oni_driver_str()
{
    return "riffa";
}

static inline oni_conf_off_t _oni_register_offset(oni_config_t reg)
{
    switch (reg) {
    case ONI_CONFIG_DEVICE_IDX:
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
    case ONI_CONFIG_SYSCLK:
        return CONFSYSCLKHZOFFSET;
    default:
        return 0;
    }
}
