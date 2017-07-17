#include "oepcie.h"

void print_state(oe_ctx* ctx){
	oe_ctx ptr = *ctx;

	printf("Printing stream paths and descriptors\n");
	printf("	Header: %s %d\n", ptr -> header.path, ptr -> header.fid);
	printf("	Config: %s %d\n", ptr -> config.path, ptr -> config.fid);
	printf("	Data: %s %d\n", ptr -> data.path, ptr -> data.fid);

	printf("Printing initialized state: %d\n", ptr -> init);
	printf("Printing config id: %d\n", ptr -> config_id);

	printf("Printing devices info:\n");
	printf("	Num_Devices: %d\n", ptr -> map.num_dev);
	for(int i = 0; i < (int)ptr -> map.num_dev; i++){
		printf("	Device %d\n", i);
		printf("		id %d", ptr -> map.devs[i].id);
		printf("		read_size %lu\n", ptr -> map.devs[i].read_size);
		printf("		read_offset %lu\n", ptr -> map.devs[i].read_offset);
	}


}

void set_get_test(oe_ctx* ctx){
	char* header_path = "test_streams/header.bin";
	char* config_path = "test_streams/config.bin";
	char* data_path = "test_streams/data.bin";

	oe_set_opt(ctx, OE_HEADER_STREAMPATH, header_path, strlen(header_path) + 1);
	oe_set_opt(ctx, OE_CONFIG_STREAMPATH, config_path, strlen(config_path) + 1);
	oe_set_opt(ctx, OE_DATA_STREAMPATH, data_path, strlen(data_path) + 1);

	printf("One\n");

	char* header_check = malloc(100); 
	char* config_check = malloc(100);
	char* data_check = malloc(100);
	size_t header_len, config_len, data_len;

	oe_get_opt(ctx, OE_HEADER_STREAMPATH, header_check, &header_len);
	oe_get_opt(ctx, OE_CONFIG_STREAMPATH, config_check, &config_len);
	oe_get_opt(ctx, OE_DATA_STREAMPATH, data_check, &data_len);

	printf("Two\n");

	printf("header path, next lines should be all zeros %s\n", (*ctx) -> header.path);
	printf("Testing\n");
	printf("%d\n", strcmp(header_path, header_check));
	printf("%d\n", strcmp(config_path, config_check));
	printf("%d\n", strcmp(data_path, data_check));

	printf("path length in next three lines\n");
	printf("%lu\n", header_len);
	printf("%lu\n", config_len);
	printf("%lu\n", data_len);

	free(header_check);
	free(config_check);
	free(data_check);

}

int main(){
	oe_ctx* ctx = oe_create_ctx();
	set_get_test(ctx);
	
	// Write our desired info into the header file
	int config_id = 333; 
	int device_one_id = 0;
	int device_two_id = 1;
	int device_three_id = 3;
	int neg_one = -1;

	int fd = open("test_streams/header.bin", O_RDWR);
	printf("fd is %d\n", fd);
	write(fd, &config_id, sizeof(int));
	write(fd, &device_one_id, sizeof(int));
	write(fd, &device_two_id, sizeof(int));
	write(fd, &device_three_id, sizeof(int));
	write(fd, &neg_one, sizeof(int));
	// Finished header file setup

	
	print_state(ctx);
	oe_init(ctx);
	print_state(ctx);

	oe_write_reg(ctx, 0, 0x0000000A, 0xABCDEF01);

	int fake_ack = 0x0000001;
	int read_fake_val = 1776;

	int fd_two = open("streams/config.bin", O_RDWR);
	lseek(fd_two, 16, SEEK_SET);
	write(fd_two, &fake_ack, sizeof(int));
	write(fd_two, &read_fake_val, sizeof(int));

	int val;
	oe_read_reg(ctx, 0, 0x0000000A, &val);
	printf("** Read value %d\n", val);

	oe_destroy_ctx(ctx);

	return 1;
}
