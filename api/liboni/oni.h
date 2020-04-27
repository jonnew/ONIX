#ifndef __ONI_H__
#define __ONI_H__

// Version macros for compile-time API version detection
#define ONI_VERSION_MAJOR 3
#define ONI_VERSION_MINOR 0
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

// OS-specific definitions
#ifdef _WIN32
#ifdef LIBONI_EXPORTS
#define ONI_EXPORT __declspec(dllexport)
#else
#define ONI_EXPORT
#endif
#else
#define ONI_EXPORT
#endif

#include "onidefs.h"

// Acqusition context
typedef struct oni_ctx_impl *oni_ctx;

// Device type
typedef struct {
    const oni_dev_id_t id;          // Device ID number (see oedevices.h)
    const oni_size_t read_size;     // Device data read size per frame in bytes
    const oni_size_t num_reads;     // Number of frames that must be read to construct a
                                    // full sample (e.g., for row reads from camera)
    const oni_size_t write_size;    // Device data write size per frame in bytes
    const oni_size_t num_writes;    // Number of frames that must be written to
                                    // construct a full output sample
} oni_device_t;

// Frame type
typedef struct oni_frame {
    const oni_fifo_dat_t dev_idx;   // Array of device indices in frame
    const oni_fifo_dat_t data_sz;   // Size in bytes of data buffer
    uint8_t *data;                  // Raw data block

} oni_frame_t;

// Context manipulation
ONI_EXPORT oni_ctx oni_create_ctx(const char* drv_name);
ONI_EXPORT int oni_init_ctx(oni_ctx ctx, int host_idx);
ONI_EXPORT int oni_destroy_ctx(oni_ctx ctx);

// Context option getting/setting
ONI_EXPORT int oni_get_opt(const oni_ctx ctx, int ctx_opt, void* value, size_t *size);
ONI_EXPORT int oni_set_opt(oni_ctx ctx, int ctx_opt, const void* value, size_t size);

// Driver option getting/setting
ONI_EXPORT int oni_get_driver_opt(const oni_ctx ctx, int drv_opt, void* value, size_t *size);
ONI_EXPORT int oni_set_driver_opt(oni_ctx ctx, int drv_opt, const void* value, size_t size);

// Hardware inspection, manipulation, and IO
ONI_EXPORT int oni_read_reg(const oni_ctx ctx, size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t *value);
ONI_EXPORT int oni_write_reg(const oni_ctx ctx, size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t value);
ONI_EXPORT int oni_read_frame(const oni_ctx ctx, oni_frame_t **frame);
ONI_EXPORT int oni_create_frame(const oni_ctx ctx, oni_frame_t **frame, size_t dev_idx, size_t data_sz);
ONI_EXPORT int oni_write_frame(const oni_ctx ctx, const oni_frame_t *frame);
ONI_EXPORT void oni_destroy_frame(oni_frame_t *frame);

// Helpers
ONI_EXPORT void oni_version(int *major, int *minor, int *patch);
ONI_EXPORT const char *oni_error_str(int err);

#ifdef __cplusplus
}
#endif

#endif
