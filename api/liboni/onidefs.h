#ifndef __ONI_DEFS_H__
#define __ONI_DEFS_H__
#include <stdint.h>

// Context options
enum {
    ONI_OPT_DEVICETABLE = 0,
    ONI_OPT_NUMDEVICES,
    ONI_OPT_RUNNING,
    ONI_OPT_RESET,
    ONI_OPT_SYSCLKHZ,
    ONI_OPT_ACQCLKHZ,
    ONI_OPT_RESETACQCOUNTER,
    ONI_OPT_HWADDRESS,
    ONI_OPT_MAXREADFRAMESIZE,
    ONI_OPT_MAXWRITEFRAMESIZE,
    ONI_OPT_BLOCKREADSIZE,
    ONI_OPT_BLOCKWRITESIZE
};

// NB: If you add an error here, make sure to update oni_error_str() in oni.c
enum {
    ONI_ESUCCESS = 0, // Success
    ONI_EPATHINVALID = -1, // Invalid stream path, fail on open
    ONI_EDEVID = -2, // Invalid device ID
    ONI_EDEVIDX = -3, // Invalid device index
    ONI_EWRITESIZE = -4, //Data size is not an integer multiple of the write size for the designated device
    ONI_EREADFAILURE = -5, // Failure to read from a stream/register
    ONI_EWRITEFAILURE = -6, // Failure to write to a stream/register
    ONI_ENULLCTX = -7, // Attempt to use a NULL context
    ONI_ESEEKFAILURE = -8, // Failure to seek on stream
    ONI_EINVALSTATE = -9, // Invalid operation for the current context run state
    ONI_EINVALOPT = -10, // Invalid context option
    ONI_EINVALARG = -11, // Invalid function arguments
    ONI_ECOBSPACK = -12, // Invalid COBS packet
    ONI_ERETRIG = -13, // Attempt to trigger an already triggered operation
    ONI_EBUFFERSIZE = -14, // Supplied buffer is too small
    ONI_EBADDEVTABLE = -15, // Badly formated device table supplied by firmware
    ONI_EBADALLOC = -16, // Bad dynamic memory allocation
    ONI_ECLOSEFAIL = -17, // File descriptor close failure, check errno
    ONI_EREADONLY = -18, // Attempted write to read only object (register, context option, etc)
    ONI_EUNIMPL = -19, // Specified, but unimplemented, feature
    ONI_EINVALREADSIZE = -20, // Block read size is smaller than the maximal read frame size
    ONI_ENOREADDEV = -21, // Frame read attempted when there are no readable devices in the device table
    ONI_EINIT = -22, // Hardware initialization failed
    ONI_EWRITEONLY = -23, // Attempted to read from a write only object (register, context option, etc)
    ONI_EINVALWRITESIZE = -24, // Write buffer pre-allocation size is smaller than the maximal write frame size
    ONI_ENOTWRITEDEV = -25, // Frame allocation attempted for a non-writable device
    ONI_EDEVIDXREPEAT = -26, // Device table contains repeated device indices

    // NB: Always at bottom
    ONI_MINERRORNUM = -27
};

// Registers available in the specification
typedef enum {
    ONI_CONFIG_DEV_IDX = 0,
    ONI_CONFIG_REG_ADDR,
    ONI_CONFIG_REG_VALUE,
    ONI_CONFIG_RW,
    ONI_CONFIG_TRIG,
    ONI_CONFIG_RUNNING,
    ONI_CONFIG_RESET,
    ONI_CONFIG_SYSCLKHZ,
    ONI_CONFIG_ACQCLKHZ,
    ONI_CONFIG_RESETACQCOUNTER,
    ONI_CONFIG_HWADDRESS
} oni_config_t;

// Fixed width device types
typedef uint32_t oni_size_t;
typedef uint32_t oni_dev_id_t; // Device IDs are 32-bit numbers
typedef uint32_t oni_dev_idx_t; // Device idx are 32-bit, byte.byte.btye.byte addresses
typedef uint32_t oni_reg_addr_t; // Registers use a 32-bit address
typedef uint32_t oni_reg_val_t;  // Registers have 32-bit values
typedef uint32_t oni_fifo_dat_t; // FIFOs use 32-bit words; // TODO: remove
typedef uint64_t oni_fifo_time_t; // FIFO bound timers use 64-bit words; // TODO: remove

#define BYTE_TO_FIFO_SHIFT 2; // TODO: remove

// Register size
#define ONI_REGSZ sizeof(oni_reg_val_t)

#endif
