#include "../liboepcie/oepcie.h"

#define NUM_DEVICES 2

char* header_path = "stream_files/header.bin";
char* config_path = "stream_files/config.bin";
char* data_path = "stream_files/data.bin";

int d_ids[NUM_DEVICES] = {0, 1}; // Two RHD2032 chips

void rhd2032_datasim(int chip_count){
	int dfid = open(data_path, O_WRONLY);

	for(int i = 0; i < chip_count; i++){
		// each channel is 16 bits = 2 butes
		for(short k = 0; k < 19; k++){
			write(dfid, &k, sizeof(short));
		}
	}

	close(dfid);
}
void stream_init(){

	int hfid = open(header_path, O_RDWR);

	if(hfid < 0)
		printf("* Open Failure\n");

	printf("opened file with %d\n", hfid);

	int ctx_configid = 230;

	int bytes = write(hfid, &ctx_configid, sizeof(int));
	if(bytes < 0){
		printf("* Write Failure %d\n", errno);
	}

	for(int i = 0; i < NUM_DEVICES; i++){
		write(hfid, &(d_ids[i]), sizeof(int));
	}
	int neg_one = -1;
	write(hfid, &neg_one, sizeof(int));
	close(hfid);
}

int main(){
	printf("Beginning test_2\n");
	stream_init();

	oe_ctx* my_ctx = oe_create_ctx();

	oe_set_opt(my_ctx, OE_HEADER_STREAMPATH, header_path, strlen(header_path) + 1);
	oe_set_opt(my_ctx, OE_CONFIG_STREAMPATH, config_path, strlen(config_path) + 1);
	oe_set_opt(my_ctx, OE_DATA_STREAMPATH, data_path, strlen(data_path) + 1);

	oe_init(my_ctx);

	oe_write_reg(my_ctx, 0, 0x1, 0x000000FF);
	oe_write_reg(my_ctx, 1, 0x2, 0x000000AA);

	int readval; 
	oe_read_reg(my_ctx, 0, 0x1, &readval);

	rhd2032_datasim(2);

	void* data = malloc(sizeof(char) * (19 * 2) * 2);
	oe_read(my_ctx, data, 19 * 2 * 2);

	for(int i = 0; i < 2; i++){
		for(int k = 0; k < 19; k++){
			printf("%d ", *(short*)(data + (k * 2)));
		}
		printf("\n");
	}

	void* device_id_array = calloc(500, sizeof(char));
	size_t length;
	oe_get_opt(my_ctx, OE_HARDWARECONFIG, device_id_array, &length);

	size_t num_dev_length;
	int num_dev;



	printf("orig %d\n", num_dev);
	oe_get_opt(my_ctx, OE_NUMDEVICES, &num_dev, &num_dev_length);
	printf("post %d\n", num_dev);
	
	printf("First id is %d\n", *((int*)(device_id_array + 0)));
	printf("2nd id is %d\n", *((int*)(device_id_array + 4)));

	void* device_off_array = calloc(500, sizeof(char));
	size_t length_off;
	oe_get_opt(my_ctx, OE_HARDWAREOFFSET, device_off_array, &length_off);

	printf("First off is %d\n", *((int*)(device_off_array + 0)));
	printf("2nd off is %d\n", *((int*)(device_off_array + 4)));

	free(device_id_array);
	free(data);
	free(device_off_array);
	

	return 1;
}
