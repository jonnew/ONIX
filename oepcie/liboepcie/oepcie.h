#ifndef OEPCIE_OEPCIE_H
#define OEPCIE_OEPCIE_H

#include <stddef.h>
#include <stdint.h>

#include "oedevice.h"

typedef enum oe_ctx_opt {
    OE_CONFIGSTREAMPATH,
    OE_DATASTREAMPATH,
    OE_SIGNALSTREAMPATH,
    OE_DEVICEMAP,
    OE_NUMDEVICES,
    OE_READFRAMESIZE,
    OE_WRITEFRAMESIZE
} oe_ctx_opt_t;

// NB: If you add an error here, make sure to update the oe_error_string array
// in oepcie.c
typedef enum oe_error {
    OE_ESUCCESS         =  0,  // Success
    OE_EPATHINVALID     = -1,  // Invalid stream path, fail on open
    OE_EREINITCTX       = -2,  // Double initialization attempt
    OE_EDEVID           = -3,  // Invalid device ID on init or reg op
    OE_EREADFAILURE     = -4,  // Failure to read from a stream/register
    OE_EWRITEFAILURE    = -5,  // Failure to write to a stream/register
    OE_ENULLCTX         = -6,  // Attempt to call function w null ctx
    OE_ESEEKFAILURE     = -7,  // Failure to seek on stream
    OE_ENOTINIT         = -8,  // Invalid operation on non-initialized ctx
    OE_EDEVIDX          = -9,  // Invalid device index
    OE_EINVALOPT        = -10, // Invalid context option
    OE_EINVALARG        = -11, // Invalid function arguments
    OE_ECANTSETOPT      = -12, // Option cannot be set in current context state
    OE_ECOBSPACK        = -13, // Invalid COBS packet
    OE_ERETRIG          = -14, // Attempt to trigger an already triggered operation
    OE_EBUFFERSIZE      = -15, // Supplied buffer is too small
    OE_EBADDEVMAP       = -16, // Badly formated device map supplied by firmware
    OE_EBADALLOC        = -17, // Bad dynamic memory allocation
    OE_ECLOSEFAIL       = -18, // File descriptor close failure, check errno
    OE_EDATATYPE        = -19, // Invalid underlying data types
    OE_EREADONLY        = -20, // Attempted write to read only object (register, context option, etc)

    // NB: Always at bottom
    OE_MINERRORNUM =    -21
} oe_error_t;

// Context
typedef struct oe_ctx_impl *oe_ctx;

// Context manipulation
oe_ctx oe_create_ctx();
int oe_init_ctx(oe_ctx ctx);
int oe_close_ctx(oe_ctx ctx);
int oe_destroy_ctx(oe_ctx ctx);
int oe_set_ctx_opt(oe_ctx ctx, const oe_ctx_opt_t option, const void* option_value, size_t option_len);
int oe_get_ctx_opt(const oe_ctx ctx, const oe_ctx_opt_t option, void* option_value, size_t *option_len);

// Hardware inspection and manipulation
int oe_read_reg(const oe_ctx ctx, const oe_dev_idx_t device_idx, const oe_reg_addr_t addr, oe_reg_val_t *value);
int oe_write_reg(const oe_ctx ctx, const oe_dev_idx_t device_idx, const oe_reg_addr_t addr, const oe_reg_val_t value);
int oe_read(const oe_ctx ctx, void *data, size_t size);

// Internal type conversion
void oe_error(oe_error_t err, char *str, size_t str_len);
int oe_device(const oe_device_id_t dev_id, char *str, size_t str_len);

#endif
