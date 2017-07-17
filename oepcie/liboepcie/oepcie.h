#ifndef _INCL_GUARD_ONE
#define _INCL_GUARD_ONE

#include "oedevices.h"

typedef enum stream_opts{
	OE_HEADER_STREAMPATH,
	OE_CONFIG_STREAMPATH,
	OE_DATA_STREAMPATH,
	OE_HARDWARECONFIG,
	OE_HARDWAREOFFSET,
	OE_NUMDEVICES
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

typedef enum initial{
 	INITIALIZED,
 	UNINITIALIZED
} init_t;

typedef struct oe_ctx_impl{
	// Three filestreams and a device map
	stream_fid_t header;
	stream_fid_t config;
	stream_fid_t data;
	dev_map_t map;

	init_t init;
	int config_id;
} oe_ctx_impl_t;

typedef struct oe_ctx_impl *oe_ctx;

int oe_init(oe_ctx* state);

oe_ctx* oe_create_ctx();

int oe_destroy_ctx(oe_ctx* state);

int oe_set_opt(oe_ctx* state, int option, const void* option_value, size_t option_len);

/*
 * For functional purposes, here we are going to assume that value points to
 * NULL, and we will allocate the string to return. It is up to the use to cl-
 * ean up memory.
 */
int oe_get_opt(const oe_ctx* state, int option, void* option_value, size_t *option_len);

int oe_write_reg(const oe_ctx* state, int device_id, int addr, int value);

int oe_read_reg(const oe_ctx* state, int device_id, int addr, int* value);

int oe_read(const oe_ctx* state, void *data, size_t size);

// int oe_write(const oe_ctx* state, void* data, size_t size);

static int oe_reg_prep(oe_ctx ctx, int device_id, int addr);

#endif