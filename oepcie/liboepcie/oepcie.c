#include "oepcie.h"

int oe_init(oe_ctx* state){
	oe_ctx ctx = *state;
	if(ctx -> init == INITIALIZED){
		return -1;
	}
	ctx -> init = INITIALIZED;

	// Successfully open all filestreams
	ctx -> header.fid = open(ctx -> header.path, O_RDONLY);
	if(ctx -> header.fid == -1){
		return -1;
	}
	ctx -> config.fid = open(ctx -> config.path, O_RDWR);
	if(ctx -> config.fid == -1){
		return -1;
	}
	ctx -> data.fid = open(ctx -> data.path, O_RDONLY);
	if(ctx -> data.fid == -1){
		return -1;
	}

	read(ctx -> header.fid, &(ctx -> config_id), sizeof(int));

	int device_id = 0;
	int num_devices = 0;

	// this for loop will not terminate if the header file is empty,
	// must have terminating -1... consider defining MAX_NUM_DEVICES
	while(device_id != -1){
		read(ctx -> header.fid, &device_id, sizeof(int));
		if(device_id > MAX_DEVICE_ID || device_id < -1){
			// //printf("Invalid device detected, returning error\n");
			return -1;
		}
		// //printf("Read device_id %d\n", device_id); // debug
		if(device_id != -1){
			num_devices++;
		}
	}
	// //printf("Devices detected: %d\n", num_devices);

	ctx -> map.num_dev = num_devices;
	ctx -> map.devs = calloc(num_devices, sizeof(device_t));

	// return to start of device_ids
	lseek(ctx -> header.fid, 4, SEEK_SET);

	for(int i = 0; i < num_devices; i++){
		int id = -1;
		if(read(ctx -> header.fid, &id, sizeof(int)) == -1){
			// //printf("*Failed on read \n");
			return -1;
		}
		ctx -> map.devs[i] = DEVICES[id];
	}

	// calculate read offsets
	for(int i = 1; i < num_devices; i++){
		ctx -> map.devs[i].read_offset = ctx -> map.devs[i - 1].read_offset +
			ctx -> map.devs[i - 1].read_size;
	}

	/*
	 * Not implemented - Write Offsets... Aren't exactly sure what/if these are
	 * used yet.
	 */

	 return 0;
}

oe_ctx* oe_create_ctx(){
	oe_ctx* ctx = malloc(sizeof(oe_ctx));
	*ctx = malloc(sizeof(struct oe_ctx_impl));
	(*ctx) -> header.path = malloc(sizeof(char));
	(*ctx) -> config.path = malloc(sizeof(char));
	(*ctx) -> data.path = malloc(sizeof(char));

	(*ctx) -> init = UNINITIALIZED;
	(*ctx) -> config_id = -1;

	return ctx;
}

int oe_destroy_ctx(oe_ctx* state){
	if(state == NULL){
		return -1;
	}

	oe_ctx ctx = *state;
	free(ctx -> header.path);
	free(ctx -> config.path);
	free(ctx -> data.path);
	free(ctx -> map.devs);
	free(ctx);
	free(state);

	return 0;
}

int oe_set_opt(oe_ctx* state, int option, const void* option_value, size_t option_len){
	oe_ctx ctx = *state;
	if(ctx -> init == INITIALIZED){
		// //printf("Already initialized, no more changing options\n");
		return -1;
	}

	switch (option){
		case OE_HEADER_STREAMPATH:
			ctx -> header.path = realloc(ctx -> header.path, option_len);
			memcpy(ctx -> header.path, value, option_len);
			break;
		case OE_CONFIG_STREAMPATH:
			ctx -> config.path = realloc(ctx -> config.path, option_len);
			memcpy(ctx -> config.path, value, option_len);
			break;
		case OE_DATA_STREAMPATH:
			ctx -> data.path = realloc(ctx -> data.path, option_len);
			memcpy(ctx -> data.path, value, option_len);
			break;
		default:
			// //printf("Improper option passed, returning with error\n");
			return -1;
			break;
	}

	return 0;
}

/*
 * For functional purposes, here we are going to assume that value points to
 * NULL, and we will allocate the string to return. It is up to the use to cl-
 * ean up memory.
 */
int oe_get_opt(const oe_ctx* state, int option, void* option_value, size_t *option_len){
	// Currently breakable with non-Cstring path
	
	switch (option){
		case OE_HEADER_STREAMPATH:
			*option_len = strlen(ctx -> header.path) + 1;
			memcpy(*option_value, ctx -> header.path, *option_len);
			break;
		case OE_CONFIG_STREAMPATH:
			*option_len = strlen(ctx -> header.config) + 1;
			memcpy(*option_value, ctx -> header.config, *option_len);
			break;
		case OE_DATA_STREAMPATH:
			*option_len = strlen(ctx -> header.data) + 1;	
			memcpy(*option_value, ctx -> header.data, *option_len);
			break;
		case OE_HARDWARECONFIG:
			// No implementation currently
			break;
		default:
			// //printf("Improper option passed, returning with error\n");
			return -1;
			break;
	}
	return 0;
}

// Checks device_id and address for register reads/writes
int _oe_reg_prep(oe_ctx_impl_t* ctx, int device_id, int addr){
	// Problem - What if we have multiple of the same devices?

	// check for whether it is a valid device
	if(device_id < 0 || device_id > MAX_DEVICE_ID){
		return -1;
	}

	// For loop checks that the device is an existing device
	int id = -1;
	for(int i = 0; i < ctx -> map.num_dev; i++){
		//printf("Device id at index %d is %d\n", i, ctx -> map.devs[i].id); // debug
		if(ctx -> map.devs[i].id == device_id){
			id = ctx -> map.devs[i].id;
		}
	}

	if(id < 0){
		//printf("(device not found)\n"); // debug
		return -1;
	}


	if(lseek(ctx -> config.fid, 0, SEEK_SET) < 0){
	 	return -1;
	}

	return 1;
}

int oe_write_reg(const oe_ctx* state, int device_id, int addr, int value){
	oe_ctx ctx = *state;
	
	if(_oe_reg_prep(ctx, device_id, addr) == -1){
		return -1;
	}

	write(ctx -> config.fid, &device_id, sizeof(int));
	write(ctx -> config.fid, &addr, sizeof(int));
	write(ctx -> config.fid, &value, sizeof(int));

	int trig = 0x00000001;
	write(ctx -> config.fid, &trig, sizeof(int));

	// At this point all write registers should be set. In our real implementation,
	// we will need to wait for a write ack here (this will be a blocking call).

	/*
	 if(lseek(ctx -> config.fid, 4, SEEK_SET) < 0){
		return -1;
	 }

	 while(read(4 bytes of ack) != 0xFFFFFFFF){}
	 */

	 return 1; 
}

int oe_read_reg(oe_ctx* state, int device_id, int addr, int* value){
	return _oe_read_reg(*state, device_id, addr, value);
}

int oe_read(oe_ctx* state, void *data, size_t size){
	return _oe_read(*state, data, size);
}

// int oe_write(const oe_ctx* state, void* data, size_t size){} 