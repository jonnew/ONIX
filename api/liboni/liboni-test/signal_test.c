#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../liboepcie/oepcie.h"
#include "../liboepcie/oepcie.c" // Access static functions for testing
#include "testfunc.h"

int main() {

    // ** COBS encoded string test ** //
    char *msg = "Hello world.";

    // Stuff
    size_t msg_len = strlen(msg) + 1;
    uint8_t en_buf[msg_len];
    int rc = oe_cobs_stuff(en_buf, (uint8_t *)msg, msg_len);
    assert(rc == 0);

    // Unstuff
    size_t buf_len = 256;
    uint8_t de_buf[buf_len];
    rc = _oe_cobs_unstuff(de_buf, en_buf, msg_len);
    char *de_msg = (char *)de_buf;
    assert(rc == 0);

    printf("Send (%zu bytes): %s\n", msg_len, msg);
    printf("Recv (%zu bytes): %s\n", strlen(de_msg) + 1, de_buf);
    assert(strcmp(msg, de_msg) == 0);

    // ** CONFIG_WACK test ** //


    printf("Success.\n");
}
