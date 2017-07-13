#include "oe_ctx_impl.h"

void _oe_create(oe_ctx_impl_t* ctx){
	ctx -> header.path = malloc(sizeof(char));
	ctx -> config.path = malloc(sizeof(char));
	ctx -> data.path = malloc(sizeof(char));

	ctx -> initialized = 'F';
	ctx -> config_id = -1;
}

int _oe_init(oe_ctx_impl_t* ctx){
	ctx -> initialized = 'T';

	// Successfully open all filestreams
	ctx -> header.fid = open(ctx -> header.path, O_RDONLY);
	if(ctx -> header.fid == -1){
		printf("Failed to open header filestream, returning with error\n");
		return -1;
	}
	ctx -> config.fid = open(ctx -> config.path, O_RDWR);
	if(ctx -> config.fid == -1){
		printf("Failed to open config filestream, returning with error\n");
		return -1;
	}
	ctx -> data.fid = open(ctx -> data.path, O_RDONLY);
	if(ctx -> data.fid == -1){
		printf("Failed to open data filestream, returning with error\n");
		return -1;
	}

	read(ctx -> header.fid, &(ctx -> config_id), sizeof(int));

	int device_id = 0;
	int num_devices = 0;

	while(device_id != -1 && ++num_devices){
		if(device_id > MAX_DEVICE_ID || device_id < 0){
			printf("Invalid device detected, returning error\n");
			return -1;
		}
		read(ctx -> header.fid, &device_id, sizeof(int));
		printf("Read device_id %d\n", device_id); // debug
	}
	printf("Devices detected: %d\n", num_devices);

	ctx -> map.num_dev = num_devices;
	ctx -> map.devs = calloc(num_devices, sizeof(device_t));

	// return to start of device_ids
	lseek(ctx -> header.fid, 4, SEEK_SET);

	for(int i = 0; i < num_devices; i++){
		int id = read(ctx -> header.fid, &id, sizeof(int));
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

	 return 1;

}

int _oe_set_opt(oe_ctx_impl_t* ctx, int option, const char* value){
	if(ctx -> initialized == 'T'){
		printf("Already initialized, no more changing options\n");
		return -1;
	}

	switch (option){
		case OE_HEADER_STREAMPATH:
			ctx -> header.path = realloc(ctx -> header.path, strlen(value) + 1);
			strcpy(ctx -> header.path, value);
			break;
		case OE_CONFIG_STREAMPATH:
			ctx -> config.path = realloc(ctx -> config.path, strlen(value) + 1);
			strcpy(ctx -> config.path, value);
			break;
		case OE_DATA_STREAMPATH:
			ctx -> data.path = realloc(ctx -> data.path, strlen(value) + 1);
			strcpy(ctx -> data.path, value);
			break;
		default:
			printf("Improper option passed, returning with error\n");
			return -1;
			break;
	}

	return 1;
}

// Allocates a buffer for value, leaves value pointing to a properly formatted
// dynamic c-string. Returns with error if option is not defined
int _oe_get_opt(oe_ctx_impl_t* ctx, int option, char* value){
	switch (option){
		case OE_HEADER_STREAMPATH:
			value = malloc(sizeof(char) * strlen(ctx -> header.path) + 1);
			strcpy(value, ctx -> header.path);
			break;
		case OE_CONFIG_STREAMPATH:
			value = malloc(sizeof(char) * strlen(ctx -> config.path) + 1);
			strcpy(value, ctx -> config.path);
			break;
		case OE_DATA_STREAMPATH:
			value = malloc(sizeof(char) * strlen(ctx -> data.path) + 1);
			strcpy(value, ctx -> data.path);
			break;
		default:
			printf("Improper option passed, returning with error\n");
			return -1;
			break;
	}
	return 1;
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
		printf("Device id at index %d is %d\n", i, ctx -> map.devs[i].id); // debug
		if(ctx -> map.devs[i].id == device_id){
			id = ctx -> map.devs[i].id;
		}
	}

	if(id < 0){
		printf("(device not found)\n"); // debug
		return -1;
	}


	// need some sort of error check on addr, just going to assume every device has 16 different addresses
	if(addr > 0xF){
	 	printf("Address not in device range\n");
	 	return -1;
	}

	if(lseek(ctx -> config.fid, 0, SEEK_SET) < 0){
	 	return -1;
	}

	return 1;
}
int _oe_write_reg(oe_ctx_impl_t* ctx, int device_id, int addr, int value){
	if(_oe_reg_prep(ctx, device_id, addr) == -1){
		return -1;
	}

	write(ctx -> config.fid, &device_id, sizeof(int));
	write(ctx -> config.fid, &addr, sizeof(int));
	write(ctx -> config.fid, &value, sizeof(int));

	int trig = 0xFFFFFFFF;
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
int _oe_read_reg(oe_ctx_impl_t* ctx, int device_id, int addr, int value){
	if(_oe_reg_prep(ctx, device_id, addr) == -1){
		return -1;
	}
	write(ctx -> config.fid, &device_id, sizeof(int));
	write(ctx -> config.fid, &addr, sizeof(int));
	if(lseek(ctx -> config.fid, 24, SEEK_SET) < 0){
		return -1;
	}
	int trig = 0xFFFFFFFF;
	write(ctx -> config.fid, &trig, sizeof(int));

	if(lseek(ctx -> config.fid, 20, SEEK_SET) < 0){
		return -1;
	}
	read(ctx -> config.fid, &value, sizeof(int));

	return 1;
}

int _oe_read(oe_ctx_impl_t* ctx, void* data, size_t size){
	read(ctx -> data.fid, &data, size);
	return 1;
}

int _oe_write(oe_ctx_impl_t* ctx, void* data, size_t size){
	return -1;
}

// Will clean up any dynamic memory we allocate, pathnames/devmap
void _oe_destroy(oe_ctx_impl_t* ctx){
	free(ctx -> header.path);
	free(ctx -> config.path);
	free(ctx -> data.path);
	free(ctx -> map.devs);
}