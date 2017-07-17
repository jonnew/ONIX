#include "oepcie.h"
#include "oepcieimpl.h"

int oe_init(oe_ctx* state){
	return _oe_init(*state);
}

oe_ctx* oe_create_ctx(){
	oe_ctx* ctx = malloc(sizeof(oe_ctx));
	*ctx = malloc(sizeof(struct oe_ctx_impl));
	_oe_create(*ctx);

	return ctx;
}

int oe_destroy_ctx(oe_ctx* state){
	_oe_destroy(*state);
	free(*state);
	free(state);
}

int oe_set_opt(oe_ctx* state, int option, const char* value){
	return _oe_set_opt(*state, option, value);
}

/*
 * For functional purposes, here we are going to assume that value points to
 * NULL, and we will allocate the string to return. It is up to the use to cl-
 * ean up memory.
 */
int oe_get_opt(oe_ctx* state, int option, char** value){
	return _oe_get_opt(*state, option, value);
}

int oe_write_reg(oe_ctx* state, int device_id, int addr, int value){
	return _oe_write_reg(*state, device_id, addr, value);
}

int oe_read_reg(oe_ctx* state, int device_id, int addr, int* value){
	return _oe_read_reg(*state, device_id, addr, value);
}

int oe_read(oe_ctx* state, void *data, size_t size){
	return _oe_read(*state, data, size);
}

int oe_write(oe_ctx* state, void* data, size_t size){
	return _oe_write(*state, data, size);
} 