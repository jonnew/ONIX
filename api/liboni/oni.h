#ifndef __ONI_H__
#define __ONI_H__

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

#ifdef LIBONI_EXPORTS
#define ONI_EXPORT __declspec(dllexport)
#else 
#define ONI_EXPORT __declspec(dllimport)
#endif
#else
#define ONI_DEFAULTCONFIGPATH  "/dev/xillybus_cmd_32"
#define ONI_DEFAULTREADPATH    "/dev/xillybus_data_read_32"
#define ONI_DEFAULTWRITEPATH   "/dev/xillybus_data_write_32"
#define ONI_DEFAULTSIGNALPATH  "/dev/xillybus_signal_8"
#define ONI_EXPORT
#endif

#include "oni-defs.h"

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

// Context
typedef struct oni_ctx_impl *oni_ctx;

// Context manipulation
ONI_EXPORT oni_ctx oni_create_ctx(const char* drivername);
ONI_EXPORT int oni_init_ctx(oni_ctx ctx, int device_index);
ONI_EXPORT int oni_destroy_ctx(oni_ctx ctx);

// Context option getting/setting
ONI_EXPORT int oni_get_opt(const oni_ctx ctx, int option, void* value, size_t *size);
ONI_EXPORT int oni_set_opt(oni_ctx ctx, int option, const void* value, size_t size);

// Driver option getting/setting
ONI_EXPORT int_oni_get_driver_opt(const oni_ctx ctx, int driver_option, void* value, size_t *size);
ONI_EXPORT int oni_set_driver_opt(oni_ctx ctx, int driver_option, const void* value, size_t size);

// Hardware inspection, manipulation, and IO
ONI_EXPORT int oni_read_reg(const oni_ctx ctx, size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t *value);
ONI_EXPORT int oni_write_reg(const oni_ctx ctx, size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t value);
ONI_EXPORT int oni_read_frame(const oni_ctx ctx, oni_frame_t **frame);
ONI_EXPORT void oni_destroy_frame(oni_frame_t *frame);
ONI_EXPORT int oni_write(const oni_ctx ctx, size_t dev_idx, const void *data, size_t data_sz);

// Internal type conversion
ONI_EXPORT void oni_version(int *major, int *minor, int *patch);
ONI_EXPORT const char *oni_error_str(int err);

#ifdef __cplusplus
}
#endif

#endif
