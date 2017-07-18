#include "../liboepcie/oepcie.h"

void set_get_test(oe_ctx* ctx){
	char* header_path = "../test/stream_files/header.bin";
	char* config_path = "../test/stream_files/config.bin";
	char* data_path = "../test/stream_files/data.bin";

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

	printf("header path, next lines should be all zeros\n");
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

	int fd = open("../test/stream_files/header.bin", O_RDWR);
	printf("fd is %d\n", fd);
	write(fd, &config_id, sizeof(int));
	write(fd, &device_one_id, sizeof(int));
	write(fd, &device_two_id, sizeof(int));
	write(fd, &device_three_id, sizeof(int));
	write(fd, &neg_one, sizeof(int));
	// Finished header file setup

	
	oe_init(ctx);


	oe_write_reg(ctx, 0, 0x0000000A, 0xABCDEF01);

	int fake_ack = 0x0000001;
	int read_fake_val = 1776;

	int fd_two = open("../test/stream_files/config.bin", O_RDWR);
	lseek(fd_two, 16, SEEK_SET);
	write(fd_two, &fake_ack, sizeof(int));
	write(fd_two, &read_fake_val, sizeof(int));

	int val;
	oe_read_reg(ctx, 0, 0x0000000A, &val);
	printf("** Read value %d\n", val);

	oe_destroy_ctx(ctx);

	return 1;
}
