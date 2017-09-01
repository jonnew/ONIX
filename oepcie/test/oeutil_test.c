#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../liboepcie/oeutil.h"

int main() {

    // ** String test ** //
    
    char *msg = "Hello world.";

    // Stuff
    size_t msg_len = strlen(msg) + 1;
    uint8_t en_buf[msg_len];
    int rc = oe_cobs_stuff(msg, msg_len, en_buf);
    assert(rc == 0);

    // Unstuff
    size_t buf_len = 256;
    char de_buf[buf_len];
    rc = oe_cobs_unstuff(en_buf, buf_len, de_buf);
    assert(rc == 0);

    printf("Send (%zu bytes): %s\n", msg_len, msg);
    printf("Recv (%zu bytes): %s\n", strlen(de_buf) + 1, de_buf);

    // ** CONFIG_WACK test ** //
}
