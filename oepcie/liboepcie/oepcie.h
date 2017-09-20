#ifndef OEPCIE_OEPCIE_H
#define OEPCIE_OEPCIE_H

#include <stddef.h>
#include <stdint.h>

#define OE_READFRAMEOVERHEAD     32 // [uint64_t sample number, (24 reserved bytes), ...]
#define OE_WRITEFRAMEOVERHEAD    32 // [( 32 reserved bytes), ...]

// Supported devices/IDs
// NB: If you add a device here, make sure to update the oe_device_string array
// in oepcie.c
typedef enum oe_device_id {
    OE_IMMEDIATEIO = 0,
    OE_RHD2032,
    OE_RHD2064,
    OE_MPU9250,

    // NB: Always on bottom
    OE_MAXDEVICEID
} oe_device_id_t;

// Fixed width device types
typedef uint32_t oe_size_t;
typedef uint32_t oe_dev_idx_t;
typedef uint32_t oe_dev_id_t;
typedef uint32_t oe_reg_addr_t;
typedef uint32_t oe_reg_val_t;

// TODO: The read/write types might be good targets for a tagged union so that
// sizeof can be used.
typedef struct oe_device {
    oe_dev_id_t id; // NB: Cannot use oe_device_id_t because this must be fixed width
    oe_size_t read_offset;
    oe_size_t read_size;
    oe_size_t write_offset;
    oe_size_t write_size;
} oe_device_t;

typedef enum oe_opt {
    OE_CONFIGSTREAMPATH,
    OE_DATASTREAMPATH,
    OE_SIGNALSTREAMPATH,
    OE_DEVICEMAP,
    OE_NUMDEVICES,
    OE_READFRAMESIZE,
    OE_WRITEFRAMESIZE,
    OE_RUNNING,
    OE_RESET,
    OE_SYSCLKHZ,
    OE_FSCLKHZ,
    OE_FSCLKM,
    OE_FSCLKD,
} oe_opt_t;

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
    OE_EINVALSTATE      = -8,  // Invalid operation for the current context run state
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
    OE_ERUNSTATESYNC    = -21, // Software and hardware run state out of sync

    // NB: Always at bottom
    OE_MINERRORNUM      = -22
} oe_error_t;

// Context
typedef struct oe_ctx_impl *oe_ctx;

// Context manipulation
oe_ctx oe_create_ctx();
int oe_init_ctx(oe_ctx ctx);
int oe_destroy_ctx(oe_ctx ctx);

// Option getting/setting
int oe_get_opt(const oe_ctx ctx, const oe_opt_t option, void* value, size_t *size);
int oe_set_opt(oe_ctx ctx, const oe_opt_t option, const void* value, size_t size);

// Hardware inspection and manipulation
int oe_read_reg(const oe_ctx ctx, const oe_dev_idx_t dev_idx, const oe_reg_addr_t addr, oe_reg_val_t *value);
int oe_write_reg(const oe_ctx ctx, const oe_dev_idx_t dev_idx, const oe_reg_addr_t addr, const oe_reg_val_t value);
int oe_read(const oe_ctx ctx, void *data, size_t size);
//int oe_write(const oe_ctx ctx, void *data, size_t size);

// Internal type conversion
int oe_error(oe_error_t err, char *str, size_t size);
int oe_device(const oe_device_id_t dev_id, char *str, size_t size);

#endif
