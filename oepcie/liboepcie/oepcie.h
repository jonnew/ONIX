#ifndef OEPCIE_H
#define OEPCIE_H

#include "oedevices.h"

typedef enum oe_stream_opts{
	OE_HEADER_STREAMPATH,
	OE_CONFIG_STREAMPATH,
	OE_DATA_STREAMPATH,
	OE_HARDWARECONFIG,
	OE_HARDWAREOFFSET,
	OE_NUMDEVICES
} oe_stream_opt_t;

typedef enum oe_error{
	OE_EHPATHINVALID = -1, // Invalid header path, fail on open
	OE_ECPATHINVALID = -2, // '' config
	OE_EDPATHINVALID = -3, // '' data
	OE_ENOTINIT, // Invalid operation on non-initialized ctx
	OE_EPREINIT = -4, // Double initialization attempt
	OE_EIDINVALID = -5, // Invalid device ID on init or reg op
	OE_EREADFAILURE = -6, // Failure to read from a stream/register
	OE_EWRITEFAILURE = -7, // Failure to write to a stream/register
	OE_ENULLCTX = -8, // Attempt to call function w null state
	OE_ENOOP = -9, // Invalid flag pass to get/set opt
	OE_ESEEKFAILURE = - 10 // Failure to seek on stream
} oe_error_t;

typedef struct oe_ctx_impl *oe_ctx;

int oe_init(oe_ctx* state);

oe_ctx* oe_create_ctx();

int oe_destroy_ctx(oe_ctx* state);

int oe_set_opt(oe_ctx* state, int option, const void* option_value, size_t option_len);

int oe_get_opt(const oe_ctx* state, int option, void* option_value, size_t *option_len);

int oe_write_reg(const oe_ctx* state, int device_idx, int addr, int value);

int oe_read_reg(const oe_ctx* state, int device_idx, int addr, int* value);

int oe_read(const oe_ctx* state, void *data, size_t size);

// int oe_write(const oe_ctx* state, void* data, size_t size);

static int oe_all_read(int fd, void* data, size_t size);
static int oe_reg_prep(oe_ctx ctx, int device_id, int addr);

// DEBUGGING FUNCTIONALITY - These are here for only debug purposes, since
// the impl information isn't provided to the user.

void OEDEBUG_print(oe_ctx* state);

#endif