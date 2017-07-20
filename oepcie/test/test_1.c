#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fcntl.h"
#include "unistd.h"

#include "../liboepcie/oepcie.h"

#define FAKE_HEADER_STREAM_FID "../test/stream_files/header.bin"
#define FAKE_CONFIG_STREAM_FID "../test/stream_files/header.bin"
#define FAKE_DATA_STREAM_FID "../test/stream_files/header.bin"
#define FAKE_SIGNAL_STREAM_FID "../test/stream_files/signal.bin"

int main(){

    // Generate context
    oe_ctx ctx = NULL;
	ctx = oe_create_ctx();

    // Set default stream paths
    const char *header_path = FAKE_HEADER_STREAM_FID;
    const char *config_path = FAKE_CONFIG_STREAM_FID;
    const char *data_path = FAKE_DATA_STREAM_FID;
    const char *signal_path = FAKE_SIGNAL_STREAM_FID;

    oe_set_ctx_opt(ctx, OE_HEADERSTREAMPATH, header_path, strlen(header_path) + 1);
	oe_set_ctx_opt(ctx, OE_CONFIGSTREAMPATH, config_path, strlen(config_path) + 1);
	oe_set_ctx_opt(ctx, OE_DATASTREAMPATH, data_path, strlen(data_path) + 1);
	oe_set_ctx_opt(ctx, OE_SIGNALSTREAMPATH, signal_path, strlen(signal_path) + 1);

    // Test oe_get_ctx_opt
    printf("\tHeader stream path set/get with too short buffer test...");
    char header_check_short[strlen(header_path) - 1];
    size_t header_len_short = strlen(header_check_short) + 1;
    assert(header_len_short != strlen(header_path) + 1);
    int rc = oe_get_ctx_opt(ctx, OE_HEADERSTREAMPATH, header_check_short, &header_len_short);
    assert(strcmp(header_path, header_check_short) != 0);
    assert(rc == OE_EINVALARG);
    printf("pass.\n");

	printf("\tHeader stream path set/get test...");
    char header_check[100];
	size_t header_len = 100;
	rc = oe_get_ctx_opt(ctx, OE_HEADERSTREAMPATH, header_check, &header_len);
    assert(rc == 0);
    assert(strcmp(header_path, header_check) == 0);
    assert(header_len == strlen(header_path) + 1);
    printf("pass.\n");

	printf("\tConfig stream path set/get test...");
    char config_check[100];
    size_t config_len = 100;
	rc = oe_get_ctx_opt(ctx, OE_CONFIGSTREAMPATH, config_check, &config_len);
    assert(rc == 0);
    assert(!strcmp(config_path, config_check));
    assert(config_len == strlen(config_path) + 1);
	printf("pass.\n");

	printf("\tData input stream path set/get test...");
    char data_check[100];
    size_t data_len = 100;
	rc = oe_get_ctx_opt(ctx, OE_DATASTREAMPATH, data_check, &data_len);
    assert(rc == 0);
    assert(!strcmp(data_path, data_check));
    assert(data_len == strlen(data_path) + 1);
	printf("pass.\n");

	// Write our desired info into the header file
	int config_id = 333;
	int device_one_id = 0;
	int device_two_id = 1;
	int device_three_id = 3;
	int neg_one = -1;

    // Generate fake header stream
    int fd_head = open(FAKE_HEADER_STREAM_FID, O_RDWR);
    printf("fd_head is %d\n", fd_head);
	write(fd_head, &config_id, sizeof(int));
	write(fd_head, &device_one_id, sizeof(int));
	write(fd_head, &device_two_id, sizeof(int));
	write(fd_head, &device_three_id, sizeof(int));
	write(fd_head, &neg_one, sizeof(int));

    // Initialize context
	assert(oe_init_ctx(ctx) == 0);

    int wack;
	oe_write_reg(ctx, 0, 0x0000000A, 0xABCDEF01, &wack);

	int fake_ack = 0x0000001;
	int read_fake_val = 1776;

	int fd_conf = open(FAKE_CONFIG_STREAM_FID, O_RDWR);
	lseek(fd_conf, 16, SEEK_SET);
	write(fd_conf, &fake_ack, sizeof(int));
	write(fd_conf, &read_fake_val, sizeof(int));

	int val;
    int rack;
	oe_read_reg(ctx, 0, 0x0000000A, &val, &rack);
	printf("** Read value %d\n", val);

	oe_destroy_ctx(ctx);

	return 1;
}
