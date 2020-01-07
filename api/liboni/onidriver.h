#ifndef __ONI_DRIVER_H__
#define __ONI_DRIVER_H__

#include <stddef.h>
#include <stdint.h>

#include "onidefs.h"

// Possible read streams
typedef enum {
    ONI_READ_STREAM_DATA = 0,
    ONI_READ_STREAM_SIGNAL
} oni_read_stream_t;

// Possible write streams.
typedef enum {
    ONI_WRITE_STREAM_DATA = 0
} oni_write_stream_t;

// Generic pointer for driver-specific options
typedef void* oni_driver_ctx;

// Prototype functions for drivers. Every driver has to implement these
#ifndef ONI_DRIVER_IGNORE_FUNCTION_PROTOTYPES // For use only for including in the main library driver loader
#ifdef _WIN32
#define ONI_DRIVER_EXPORT __declspec(dllexport)
#else
#define ONI_DRIVER_EXPORT
#endif

ONI_DRIVER_EXPORT oni_driver_ctx oni_driver_create_ctx();
ONI_DRIVER_EXPORT int oni_driver_destroy_ctx(oni_driver_ctx);

// Initialize driver. Argument is the host device index
ONI_DRIVER_EXPORT int oni_driver_init(oni_driver_ctx driver_ctx, int host_idx);
ONI_DRIVER_EXPORT int oni_driver_read_stream(oni_driver_ctx driver_ctx, oni_read_stream_t stream, void* data, size_t size);
ONI_DRIVER_EXPORT int oni_driver_write_stream(oni_driver_ctx driver_ctx, oni_write_stream_t stream, const char* data, size_t size);
ONI_DRIVER_EXPORT int oni_driver_read_config(oni_driver_ctx driver_ctx, oni_config_t config, oni_reg_val_t* value);
ONI_DRIVER_EXPORT int oni_driver_write_config(oni_driver_ctx driver_ctx, oni_config_t config, oni_reg_val_t value);

// This gets called when oni_set_opt is called. This method does not need to
// perform any configuration but it is provided for the driver to do some
// internal adjustments if required
ONI_DRIVER_EXPORT int oni_driver_set_opt_callback(oni_driver_ctx driver_ctx, int oni_option, const void* value, size_t option_len);

// Functions to get and set set driver-specific options. This kind of optiosn
// must be avoided when necessary to allow for a general interface
ONI_DRIVER_EXPORT int oni_driver_set_opt(oni_driver_ctx driver_ctx, int driver_option, const void* value, size_t option_len);
ONI_DRIVER_EXPORT int oni_driver_get_opt(oni_driver_ctx driver_ctx, int driver_option, void* value, size_t* option_len);

// Get a string identifying the driver
ONI_DRIVER_EXPORT const char* oni_driver_str();

#endif

#endif
