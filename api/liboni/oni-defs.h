#ifndef __ONI_DEFS_H__
#define __ONI_DEFS_H__
#include <stdint.h>

// Context options
enum {
	ONI_DEVICEMAP,
	ONI_NUMDEVICES,
	ONI_MAXREADFRAMESIZE,
	ONI_RUNNING,
	ONI_RESET,
	ONI_SYSCLKHZ,
	ONI_BLOCKREADSIZE
};

// NB: If you add an error here, make sure to update oni_error_str()
enum {
	ONI_ESUCCESS = 0,  // Success
	ONI_EPATHINVALID = -1,  // Invalid stream path, fail on open
	ONI_EDEVID = -2,  // Invalid device ID on init or reg op
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
	ONI_EINVALREADSIZE = -20, // Block read size is smaller than the maximal frame size
	ONI_ENOREADDEV = -21, // Frame read attempted when there are no readable devices in the device map

	// NB: Always at bottom
	ONI_MINERRORNUM = -22
};

// Registers available in the specification
typedef enum {
	ONI_CONFIG_DEVICE_IDX = 0,
	ONI_CONFIG_REG_ADDR,
	ONI_CONFIG_REG_VALUE,
	ONI_CONFIG_RW,
	ONI_CONFIG_TRIG,
	ONI_CONFIG_RUNNING,
	ONI_RUNONRESET,
	ONI_CONFIG_RESET,
	ONI_CONFIG_SYSCLK
} oni_config_t;

// Fixed width device types
typedef uint32_t oni_size_t;
typedef uint32_t oni_dev_id_t;
typedef uint32_t oni_reg_addr_t;
typedef uint32_t oni_reg_val_t;

// Register size
#define ONI_REGSZ sizeof(oni_reg_val_t)

#endif
