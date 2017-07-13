#ifndef _INCL_GUARD_TWO
#define _INCL_GUARD_TWO

#include "devices.h"

typedef enum stream_opts{
	OE_HEADER_STREAMPATH,
	OE_CONFIG_STREAMPATH,
	OE_DATA_STREAMPATH
} stream_opt_t;

/*
 * Stores filestream info
 */
typedef struct stream_fid{
	char* path;
	int fid;
} stream_fid_t;

/*
 * Map of all devices
 */
typedef struct dev_map{
	int num_dev;
	device_t* devs;
} dev_map_t;

typedef struct oe_ctx_impl{
	// Three filestreams and a device map
	stream_fid_t header;
	stream_fid_t config;
	stream_fid_t data;
	dev_map_t map;

	char initialized; 
	int config_id;
} oe_ctx_impl_t;

// helper function for oe_create(), allocates chars so we can use realloc in
// setopt
void _oe_create(oe_ctx_impl_t* ctx);

// Will initialize the filestreams, allocate pathnames so that we can use
// realloc() in set_opt, reads header stream to initialize devices
int _oe_init(oe_ctx_impl_t* ctx);

// Requires value is a properly formatted c_string, and will return with
// error if option is not defined
int _oe_set_opt(oe_ctx_impl_t* ctx, int option, const char* value);

// Allocates a buffer for value, leaves value pointing to a properly formatted
// dynamic c-string. Returns with error if option is not defined
int _oe_get_opt(oe_ctx_impl_t* ctx, int option, char* value);

int _oe_write_reg(oe_ctx_impl_t* ctx, int device_id, int addr, int value);
int _oe_read_reg(oe_ctx_impl_t* ctx, int device_id, int addr, int value);

int _oe_read(oe_ctx_impl_t* ctx, void* data, size_t size);
int _oe_write(oe_ctx_impl_t* ctx, void* data, size_t size);

// Will clean up any dynamic memory we allocate, pathnames/devmap
void _oe_destroy(oe_ctx_impl_t* ctx);

#endif


