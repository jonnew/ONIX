#ifndef __ONI_DRIVER_LOADER_H__
#define __ONI_DRIVER_LOADER_H__

#define ONI_DRIVER_IGNORE_FUNCTION_PROTOTYPES
#include "oni-driver.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
typedef HINSTANCE lib_handle_t;
#else
#include <dlfcn.h>
#include <execinfo.h>
typedef void* lib_handle_t;
#endif

// Function pointers for common access

typedef oni_driver_ctx(*oni_driver_create_ctx_f)();
typedef int(*oni_driver_destroy_ctx_f)(oni_driver_ctx);

typedef int(*oni_driver_init_f)(oni_driver_ctx, int);
typedef int(*oni_driver_read_stream_f)(oni_driver_ctx, oni_read_stream_t, void*, size_t);
typedef int(*oni_driver_write_stream_f)(oni_driver_ctx, oni_write_stream_t, const char*, size_t);
typedef int(*oni_driver_read_config_f)(oni_driver_ctx, oni_config_t, oni_reg_val_t*);
typedef int(*oni_driver_write_config_f)(oni_driver_ctx, oni_config_t, oni_reg_val_t);

typedef int(*oni_driver_set_opt_callback_f)(oni_driver_ctx, int, const void*, size_t);


typedef int(*oni_driver_set_opt_f)(oni_driver_ctx, int, const void*, size_t);
typedef int(*oni_driver_get_opt_f)(oni_driver_ctx, int, void*, size_t*);

typedef const char*(*oni_driver_get_id_f)();

// Driver field with function table and driver context
typedef struct oni_driver {
	lib_handle_t handle;
	oni_driver_ctx ctx;
	oni_driver_create_ctx_f create_ctx;
	oni_driver_destroy_ctx_f destroy_ctx;
	oni_driver_init_f init;
	oni_driver_read_stream_f read_stream;
	oni_driver_write_stream_f write_stream;
	oni_driver_read_config_f read_config;
	oni_driver_write_config_f write_config;
	oni_driver_set_opt_callback_f set_opt_callback;
	oni_driver_set_opt_f set_opt;
	oni_driver_get_opt_f get_opt;
	oni_driver_get_id_f get_id;
} oni_driver_t;

int oni_create_driver(const char* lib_name, oni_driver_t* driver);
int oni_destroy_driver(oni_driver_t* driver);

#endif // !__ONI_DRIVER_LOADER_H__