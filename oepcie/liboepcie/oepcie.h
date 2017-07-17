#ifndef _INCL_GUARD_ONE
#define _INCL_GUARD_ONE

typedef struct oe_ctx_impl *oe_ctx;

int oe_init(oe_ctx* ctx);

oe_ctx* oe_create_ctx();

void oe_destroy_ctx(oe_ctx* state);

int oe_set_opt(oe_ctx* state, int option, const char* value);

/*
 * For functional purposes, here we are going to assume that value points to
 * NULL, and we will allocate the string to return. It is up to the use to cl-
 * ean up memory.
 */
int oe_get_opt(oe_ctx* state, int option, char** value);

int oe_write_reg(oe_ctx* state, int device_id, int addr, int value);

int oe_read_reg(oe_ctx* state, int device_id, int addr, int* value);

int oe_read(oe_ctx* state, void *data, size_t size);

int oe_write(oe_ctx* state, void* data, size_t size);

#endif