#ifndef __OEPCIE_H__
#define __OEPCIE_H__

// Version macros for compile-time API version detection
#define OE_VERSION_MAJOR 2
#define OE_VERSION_MINOR 2
#define OE_VERSION_PATCH 0

#define OE_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define OE_VERSION \
    OE_MAKE_VERSION(OE_VERSION_MAJOR, OE_VERSION_MINOR, OE_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// Read frame constants
#define OE_RFRAMEHEADERSZ     32 // [uint64_t host_clock, uint16_t n_devs, uint8_t frame_err, (22 reserved bytes), ...]
#define OE_RFRAMESAMPLEOFF    0  // Read frame host clock tick offset
#define OE_RFRAMENDEVOFF      8  // Read frame number of devices offset
#define OE_RFRAMENERROFF      10 // Read frame error offset

// OS-specific definitions
#ifdef _WIN32
#define OE_DEFAULTCONFIGPATH  "\\\\.\\xillybus_cmd_32"
#define OE_DEFAULTREADPATH    "\\\\.\\xillybus_data_read_32"
#define OE_DEFAULTWRITEPATH   "\\\\.\\xillybus_data_write_32"
#define OE_DEFAULTSIGNALPATH  "\\\\.\\xillybus_signal_8"
#define OE_EXPORT __declspec(dllexport)
#else
#define OE_DEFAULTCONFIGPATH  "/dev/xillybus_cmd_32"
#define OE_DEFAULTREADPATH    "/dev/xillybus_data_read_32"
#define OE_DEFAULTWRITEPATH   "/dev/xillybus_data_write_32"
#define OE_DEFAULTSIGNALPATH  "/dev/xillybus_signal_8"
#define OE_EXPORT
#endif

// Fixed width device types
typedef uint32_t oe_size_t;
typedef uint32_t oe_dev_id_t;
typedef uint32_t oe_reg_addr_t;
typedef uint32_t oe_reg_val_t;

// Device type
typedef struct {
    oe_dev_id_t id;         // Device ID number (see oedevices.h)
    oe_size_t read_size;    // Device data read size per frame in bytes
    oe_size_t num_reads;    // Number of frames that must be read to construct a
                            // full sample (e.g., for row reads from camera)
    oe_size_t write_size;   // Device data write size per frame in bytes
    oe_size_t num_writes;   // Number of frames that must be written to construct
                            // a full output sample
} oe_device_t;

// Opaque handle to reference counting buffer.
typedef struct oe_buf_impl *oe_buffer;

// Frame type
typedef struct oe_frame {

    // Header
    uint64_t clock;         // Base clock counter
    uint16_t num_dev;       // Number of devices in frame
    uint8_t corrupt;        // Is this frame corrupt?

    // Data
    oe_size_t *dev_idxs;    // Array of device indices in frame
    oe_size_t *dev_offs;    // Device data offsets within data block
    uint8_t *data;          // Multi-device raw data block
    oe_size_t data_sz;      // Size in bytes of data buffer

    // External buffer, don't touch
    oe_buffer buffer;       // Handle to external buffer

} oe_frame_t;

// Context options
enum {
    OE_CONFIGSTREAMPATH,
    OE_READSTREAMPATH,
    OE_WRITESTREAMPATH,
    OE_SIGNALSTREAMPATH,
    OE_DEVICEMAP,
    OE_NUMDEVICES,
    OE_MAXREADFRAMESIZE,
    OE_MAXWRITEFRAMESIZE,
    OE_RUNNING,
    OE_RESET,
    OE_SYSCLKHZ,
    OE_ACQCLKHZ,
    OE_BLOCKREADSIZE
};

// NB: If you add an error here, make sure to update oe_error_str()
enum {
    OE_ESUCCESS         =  0,  // Success
    OE_EPATHINVALID     = -1,  // Invalid stream path, fail on open
    OE_EDEVID           = -2,  // Invalid device ID on init or reg op
    OE_EDEVIDX          = -3,  // Invalid device index
    OE_EWRITESIZE       = -4,  // Data write size is incorrect for designated device
    OE_EREADFAILURE     = -5,  // Failure to read from a stream/register
    OE_EWRITEFAILURE    = -6,  // Failure to write to a stream/register
    OE_ENULLCTX         = -7,  // Attempt to call function w null ctx
    OE_ESEEKFAILURE     = -8,  // Failure to seek on stream
    OE_EINVALSTATE      = -9,  // Invalid operation for the current context run state
    OE_EINVALOPT        = -10, // Invalid context option
    OE_EINVALARG        = -11, // Invalid function arguments
    OE_ECOBSPACK        = -12, // Invalid COBS packet
    OE_ERETRIG          = -13, // Attempt to trigger an already triggered operation
    OE_EBUFFERSIZE      = -14, // Supplied buffer is too small
    OE_EBADDEVMAP       = -15, // Badly formated device map supplied by firmware
    OE_EBADALLOC        = -16, // Bad dynamic memory allocation
    OE_ECLOSEFAIL       = -17, // File descriptor close failure, check errno
    OE_EREADONLY        = -18, // Attempted write to read only object (register, context option, etc)
    OE_EUNIMPL          = -19, // Specified, but unimplemented, feature
    OE_EINVALREADSIZE   = -20, // Block read size is smaller than the maximal frame size

    // NB: Always at bottom
    OE_MINERRORNUM      = -21
};

// Context
typedef struct oe_ctx_impl *oe_ctx;

// Context manipulation
OE_EXPORT oe_ctx oe_create_ctx();
OE_EXPORT int oe_init_ctx(oe_ctx ctx);
OE_EXPORT int oe_destroy_ctx(oe_ctx ctx);

// Context option getting/setting
OE_EXPORT int oe_get_opt(const oe_ctx ctx, int option, void* value, size_t *size);
OE_EXPORT int oe_set_opt(oe_ctx ctx, int option, const void* value, size_t size);

// Hardware inspection, manipulation, and IO
OE_EXPORT int oe_read_reg(const oe_ctx ctx, size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t *value);
OE_EXPORT int oe_write_reg(const oe_ctx ctx, size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t value);
OE_EXPORT int oe_read_frame(const oe_ctx ctx, oe_frame_t **frame);
OE_EXPORT void oe_destroy_frame(oe_frame_t *frame);
OE_EXPORT int oe_write(const oe_ctx ctx, size_t dev_idx, void *data, size_t data_sz);

// Internal type conversion
OE_EXPORT void oe_version(int *major, int *minor, int *patch);
OE_EXPORT const char *oe_error_str(int err);

#ifdef __cplusplus
}
#endif

#endif
