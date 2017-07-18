#ifndef OEPCIE_C
#define OEPCIE_C
#include "oepcie.h"

typedef struct stream_fid{
	char* path;
	int fid;
} stream_fid_t;

typedef struct dev_map{
	int num_dev;
	device_t* devs;
} dev_map_t;

typedef enum initial{
 	INITIALIZED,
 	UNINITIALIZED
} init_t;

typedef struct oe_ctx_impl{
	stream_fid_t header;
	stream_fid_t config;
	stream_fid_t data;
	dev_map_t map;

	init_t init;
	int config_id;
} oe_ctx_impl_t;


int oe_init(oe_ctx* state){
	oe_ctx ctx = *state;
	if(ctx -> init == INITIALIZED)
		return OE_EPREINIT;

	// Successfully open all filestreams
	ctx -> header.fid = open(ctx -> header.path, O_RDONLY);
	if(ctx -> header.fid == -1)
		return OE_EHPATHINVALID;
	ctx -> config.fid = open(ctx -> config.path, O_RDWR);
	if(ctx -> config.fid == -1)
		return OE_ECPATHINVALID;
	ctx -> data.fid = open(ctx -> data.path, O_RDONLY);
	if(ctx -> data.fid == -1)
		return OE_EDPATHINVALID;

	read(ctx -> header.fid, &(ctx -> config_id), sizeof(int));

	int device_id = 0;
	int num_dev = 0;

	// this for loop will not terminate if the header file is empty,
	// must have terminating -1... consider defining MAX_num_dev
	while(device_id != -1){
		read(ctx -> header.fid, &device_id, sizeof(int));
		if(device_id > MAX_DEVICE_ID || device_id < -1)
			return OE_EIDINVALID;
		if(device_id != -1)
			num_dev++;
	}

	ctx -> map.num_dev = num_dev;
	ctx -> map.devs = calloc(num_dev, sizeof(device_t));

	// return to start of device_ids
	lseek(ctx -> header.fid, 4, SEEK_SET);

	for(int i = 0; i < num_dev; i++){
		int id = -1;
		if(read(ctx -> header.fid, &id, sizeof(int)) == -1)
			return OE_EREADFAILURE;
		ctx -> map.devs[i] = DEVICES[id];
	}

	// calculate read offsets
	for(int i = 1; i < num_dev; i++){
		ctx -> map.devs[i].read_offset = ctx -> map.devs[i - 1].read_offset +
			ctx -> map.devs[i - 1].read_size;
	}

	/*
	 * Not implemented - Write Offsets... Aren't exactly sure what/if these are
	 * used yet.
	 */

	ctx -> init = INITIALIZED;

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
	if(state == NULL)
		return OE_ENULLCTX;

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
	if(ctx -> init == INITIALIZED)
		return OE_EPREINIT;

	switch (option){
		case OE_HEADER_STREAMPATH:
			ctx -> header.path = realloc(ctx -> header.path, option_len);
			memcpy(ctx -> header.path, option_value, option_len);
			break;
		case OE_CONFIG_STREAMPATH:
			ctx -> config.path = realloc(ctx -> config.path, option_len);
			memcpy(ctx -> config.path, option_value, option_len);
			break;
		case OE_DATA_STREAMPATH:
			ctx -> data.path = realloc(ctx -> data.path, option_len);
			memcpy(ctx -> data.path, option_value, option_len);
			break;
		default:
			// //printf("Improper option passed, returning with error\n");
			return OE_ENOOP;
			break;
	}

	return 0;
}

int oe_get_opt(const oe_ctx* state, int option, void* option_value, size_t *option_len){
	// Currently breakable with non-Cstring path
	oe_ctx ctx = *state;

	switch (option){
		case OE_HEADER_STREAMPATH:
			*option_len = strlen(ctx -> header.path) + 1;
			memcpy(option_value, ctx -> header.path, *option_len);
			break;
		case OE_CONFIG_STREAMPATH:
			*option_len = strlen(ctx -> config.path) + 1;
			memcpy(option_value, ctx -> config.path, *option_len);
			break;
		case OE_DATA_STREAMPATH:
			*option_len = strlen(ctx -> data.path) + 1;	
			memcpy(option_value, ctx -> data.path, *option_len);
			break;
		case OE_HARDWARECONFIG:
			// No implementation currently
			if(ctx -> init == UNINITIALIZED)
				return OE_ENOTINIT;
			*option_len = ctx -> map.num_dev * sizeof(int);
			for(int i = 0; i < ctx -> map.num_dev; i++){
				memcpy(option_value + (i * sizeof(int)), &(ctx -> map.devs[i].id), sizeof(int));
			}
			break;
		case OE_HARDWAREOFFSET:
			if(ctx -> init == UNINITIALIZED)
				return OE_ENOTINIT;
			*option_len = ctx -> map.num_dev * sizeof(int);
			for(int i = 0; i < ctx -> map.num_dev; i++){
				memcpy(option_value + (i * sizeof(int)), &(ctx -> map.devs[i].read_offset), sizeof(int));
			}
			break;
		case OE_NUMDEVICES:
			memcpy(option_value, &(ctx -> map.num_dev), sizeof(int));
			*option_len = sizeof(int);
			break;
		default:
			//printf("Improper option passed, returning with error\n");
			return -1;
			break;
	}
	return 0;
}

// Checks device_id and address for register reads/writes
static int oe_reg_prep(oe_ctx_impl_t* ctx, int device_idx, int addr){
	// Checks that the device index is valid
	if(device_idx >= ctx -> map.num_dev || device_idx < 0)
		return OE_EIDINVALID;
	if(lseek(ctx -> config.fid, 0, SEEK_SET) < 0)
	 	return OE_ESEEKFAILURE;

	return 0;
}

int oe_write_reg(const oe_ctx* state, int device_idx, int addr, int value){
	oe_ctx ctx = *state;

	int prep = oe_reg_prep(ctx, device_idx, addr);
	if(prep == -1)
		return prep;

	write(ctx -> config.fid, &device_idx, sizeof(int));
	write(ctx -> config.fid, &addr, sizeof(int));
	write(ctx -> config.fid, &value, sizeof(int));

	int trig = 0x00000001;
	write(ctx -> config.fid, &trig, sizeof(int));

	// At this point all write registers should be set. In our real implementation,
	// we will need to wait for a write ack here (this will be a blocking call).

	 return 0; 
}

int oe_read_reg(const oe_ctx* state, int device_idx, int addr, int* value){
	oe_ctx ctx = *state;
	
	int prep = oe_reg_prep(ctx, device_idx, addr);
	if(prep == -1)
		return prep;

	write(ctx -> config.fid, &device_idx, sizeof(int));
	write(ctx -> config.fid, &addr, sizeof(int));
	if(lseek(ctx -> config.fid, 24, SEEK_SET) < 0)
		return OE_ESEEKFAILURE;
	int trig = 0x00000001;
	write(ctx -> config.fid, &trig, sizeof(int));

	if(lseek(ctx -> config.fid, 20, SEEK_SET) < 0)
		return OE_ESEEKFAILURE;

	// Will need to wait for ACK

	read(ctx -> config.fid, value, sizeof(int));


	return 0;
}

static int oe_all_read(int fd, void* data, size_t size){
	int received = 0;
	int rc;

	while (received < size) {
    	rc = read(fd, data + received, size - received);
    	if (rc < 0) {
      		return OE_EREADFAILURE; 
    	}
    	if (rc == 0) {
      		return OE_EREADFAILURE;
    	}
  	 	received += rc;
   	}
   	return 0;
}

int oe_read(const oe_ctx* state, void *data, size_t size){
	oe_ctx ctx = *state;
	return oe_all_read(ctx -> data.fid, data, size);
}

// int oe_write(const oe_ctx* state, void* data, size_t size){} 
#endif