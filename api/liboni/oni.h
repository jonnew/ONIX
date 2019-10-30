#ifndef __OEPCIE_H__
#define __OEPCIE_H__

// Version macros for compile-time API version detection
#define ONI_VERSION_MAJOR 2
#define ONI_VERSION_MINOR 3
#define ONI_VERSION_PATCH 0

#define ONI_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define ONI_VERSION \
    ONI_MAKE_VERSION(ONI_VERSION_MAJOR, ONI_VERSION_MINOR, ONI_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// Read frame constants
#define ONI_RFRAMEHEADERSZ     32 // [uint64_t host_clock, uint16_t n_devs, uint8_t frame_err, (22 reserved bytes), ...]
#define ONI_RFRAMESAMPLEOFF    0  // Read frame host clock tick offset
#define ONI_RFRAMENDEVOFF      8  // Read frame number of devices offset
#define ONI_RFRAMENERROFF      10 // Read frame error offset

// OS-specific definitions
#ifdef _WIN32
#define ONI_DEFAULTCONFIGPATH  "\\\\.\\xillybus_cmd_32"
#define ONI_DEFAULTREADPATH    "\\\\.\\xillybus_data_read_32"
#define ONI_DEFAULTWRITEPATH   "\\\\.\\xillybus_data_write_32"
#define ONI_DEFAULTSIGNALPATH  "\\\\.\\xillybus_signal_8"

#define ONI_EXPORT __declspec(dllexport)
#else
#define ONI_DEFAULTCONFIGPATH  "/dev/xillybus_cmd_32"
#define ONI_DEFAULTREADPATH    "/dev/xillybus_data_read_32"
#define ONI_DEFAULTWRITEPATH   "/dev/xillybus_data_write_32"
#define ONI_DEFAULTSIGNALPATH  "/dev/xillybus_signal_8"
#define ONI_EXPORT
#endif

// Fixed width device types
typedef uint32_t oni_size_t;
typedef uint32_t oni_dev_id_t;
typedef uint32_t oni_reg_addr_t;
typedef uint32_t oni_reg_val_t;

// Device type
typedef struct {
    oni_dev_id_t id;         // Device ID number (see oedevices.h)
    oni_size_t read_size;    // Device data read size per frame in bytes
    oni_size_t num_reads;    // Number of frames that must be read to construct a
                            // full sample (e.g., for row reads from camera)
    oni_size_t write_size;   // Device data write size per frame in bytes
    oni_size_t num_writes;   // Number of frames that must be written to construct
                            // a full output sample
} oni_device_t;

// Opaque handle to reference counting buffer.
typedef struct oni_buf_impl *oni_buffer;

// Frame type
typedef struct oni_frame {

    // Header
    uint64_t clock;         // Base clock counter
    uint16_t num_dev;       // Number of devices in frame
    uint8_t corrupt;        // Is this frame corrupt?

    // Data
    oni_size_t *dev_idxs;    // Array of device indices in frame
    oni_size_t *dev_offs;    // Device data offsets within data block
    uint8_t *data;          // Multi-device raw data block
    oni_size_t data_sz;      // Size in bytes of data buffer

    // External buffer, don't touch
    oni_buffer buffer;       // Handle to external buffer

} oni_frame_t;

// Context options
enum {
    ONI_CONFIGSTREAMPATH,
    ONI_READSTREAMPATH,
    ONI_WRITESTREAMPATH,
    ONI_SIGNALSTREAMPATH,
    ONI_DEVICEMAP,
    ONI_NUMDEVICES,
    ONI_MAXREADFRAMESIZE,
    ONI_RUNNING,
    ONI_RESET,
    ONI_SYSCLKHZ,
    ONI_ACQCLKHZ,
    ONI_BLOCKREADSIZE
};

// NB: If you add an error here, make sure to update oni_error_str()
enum {
    ONI_ESUCCESS         =  0,  // Success
    ONI_EPATHINVALID     = -1,  // Invalid stream path, fail on open
    ONI_EDEVID           = -2,  // Invalid device ID on init or reg op
    ONI_EDEVIDX          = -3,  // Invalid device index
    ONI_EWRITESIZE       = -4,  // Data write size is incorrect for designated device
    ONI_EREADFAILURE     = -5,  // Failure to read from a stream/register
    ONI_EWRITEFAILURE    = -6,  // Failure to write to a stream/register
    ONI_ENULLCTX         = -7,  // Attempt to call function w null ctx
    ONI_ESEEKFAILURE     = -8,  // Failure to seek on stream
    ONI_EINVALSTATE      = -9,  // Invalid operation for the current context run state
    ONI_EINVALOPT        = -10, // Invalid context option
    ONI_EINVALARG        = -11, // Invalid function arguments
    ONI_ECOBSPACK        = -12, // Invalid COBS packet
    ONI_ERETRIG          = -13, // Attempt to trigger an already triggered operation
    ONI_EBUFFERSIZE      = -14, // Supplied buffer is too small
    ONI_EBADDEVMAP       = -15, // Badly formated device map supplied by firmware
    ONI_EBADALLOC        = -16, // Bad dynamic memory allocation
    ONI_ECLOSEFAIL       = -17, // File descriptor close failure, check errno
    ONI_EREADONLY        = -18, // Attempted write to read only object (register, context option, etc)
    ONI_EUNIMPL          = -19, // Specified, but unimplemented, feature
    ONI_EINVALREADSIZE   = -20, // Block read size is smaller than the maximal frame size
    ONI_ENOREADDEV       = -21, // Frame read attempted when there are no readable devices in the device map

    // NB: Always at bottom
    ONI_MINERRORNUM      = -22
};

// Context
typedef struct oni_ctx_impl *oni_ctx;

// Context manipulation
ONI_EXPORT oni_ctx oni_create_ctx();
ONI_EXPORT int oni_init_ctx(oni_ctx ctx);
ONI_EXPORT int oni_destroy_ctx(oni_ctx ctx);

// Context option getting/setting
ONI_EXPORT int oni_get_opt(const oni_ctx ctx, int option, void* value, size_t *size);
ONI_EXPORT int oni_set_opt(oni_ctx ctx, int option, const void* value, size_t size);

// Hardware inspection, manipulation, and IO
ONI_EXPORT int oni_read_reg(const oni_ctx ctx, size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t *value);
ONI_EXPORT int oni_write_reg(const oni_ctx ctx, size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t value);
ONI_EXPORT int oni_read_frame(const oni_ctx ctx, oni_frame_t **frame);
ONI_EXPORT void oni_destroy_frame(oni_frame_t *frame);
ONI_EXPORT int oni_write(const oni_ctx ctx, size_t dev_idx, void *data, size_t data_sz);

// Internal type conversion
ONI_EXPORT void oni_version(int *major, int *minor, int *patch);
ONI_EXPORT const char *oni_error_str(int err);

#ifdef __cplusplus
}
#endif

#endif
