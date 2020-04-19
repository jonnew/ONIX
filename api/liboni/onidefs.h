#ifndef __ONI_DEFS_H__
#define __ONI_DEFS_H__
#include <stdint.h>

// Context options
enum {
    ONI_OPT_DEVICEMAP = 0,
    ONI_OPT_NUMDEVICES,
    ONI_OPT_MAXREADFRAMESIZE,
    ONI_OPT_RUNONRESET,
    ONI_OPT_RUNNING,
    ONI_OPT_RESET,
    ONI_OPT_SYSCLKHZ,
    ONI_OPT_BLOCKREADSIZE,
    ONI_OPT_BLOCKWRITESIZE
};

// NB: If you add an error here, make sure to update oni_error_str() in oni.c
enum {
    ONI_ESUCCESS = 0,  // Success
    ONI_EPATHINVALID = -1,  // Invalid stream path, fail on open
    ONI_EDEVID = -2,  // Invalid device ID
    ONI_EDEVIDX = -3,  // Invalid device index
    ONI_EWRITESIZE = -4,  // Data write size is incorrect for designated device
    ONI_EREADFAILURE = -5,  // Failure to read from a stream/register
    ONI_EWRITEFAILURE = -6,  // Failure to write to a stream/register
    ONI_ENULLCTX = -7,  // Attempt to call function w null ctx
    ONI_ESEEKFAILURE = -8,  // Failure to seek on stream
    ONI_EINVALSTATE = -9,  // Invalid operation for the current context run state
    ONI_EINVALOPT = -10, // Invalid context option
    ONI_EINVALARG = -11, // Invalid function arguments
    ONI_ECOBSPACK = -12, // Invalid COBS packet
    ONI_ERETRIG = -13, // Attempt to trigger an already triggered operation
    ONI_EBUFFERSIZE = -14, // Supplied buffer is too small
    ONI_EBADDEVMAP = -15, // Badly formated device map supplied by firmware
    ONI_EBADALLOC = -16, // Bad dynamic memory allocation
    ONI_ECLOSEFAIL = -17, // File descriptor close failure, check errno
    ONI_EREADONLY = -18, // Attempted write to read only object (register, context option, etc)
    ONI_EUNIMPL = -19, // Specified, but unimplemented, feature
    ONI_EINVALREADSIZE = -20, // Block read size is smaller than the maximal read frame size
    ONI_ENOREADDEV = -21, // Frame read attempted when there are no readable devices in the device map
    ONI_EINIT = -22, // Hardware initialization failed
    ONI_EWRITEONLY = -23, // Attempted to read from a write only object (register, context option, etc)
    ONI_EINVALWRITESIZE = -24, // Write buffer pre-allocation size is smaller than the maximal write frame size
    ONI_ENOWRITEDEV = -25, // Frame allocation attempted when there are no writable devices in the device map

    // NB: Always at bottom
    ONI_MINERRORNUM = -26
};

// Registers available in the specification
typedef enum {
    ONI_CONFIG_DEVICE_IDX = 0,
    ONI_CONFIG_REG_ADDR,
    ONI_CONFIG_REG_VALUE,
    ONI_CONFIG_RW,
    ONI_CONFIG_TRIG,
    ONI_CONFIG_RUNNING,
    ONI_CONFIG_RESET,
    ONI_CONFIG_SYSCLK
} oni_config_t;

// Fixed width device types
typedef uint32_t oni_size_t;
typedef uint32_t oni_dev_id_t; // Device IDs are 32-bit numbers
typedef uint32_t oni_reg_addr_t; // Registers use a 32-bit address
typedef uint32_t oni_reg_val_t;  // Registers have 32-bit values
typedef uint32_t oni_fifo_dat_t; // FIFOs use 32-bit words; // TODO: remove

#define BYTE_TO_FIFO_SHIFT 2; // TODO: remove

// Register size
#define ONI_REGSZ sizeof(oni_reg_val_t)

#endif
