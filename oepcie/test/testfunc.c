#include <stddef.h>
#include <stdint.h>
#include "testfunc.h"

#define FINISHBLOCK(X) (*code_ptr = (X), code_ptr = dst++, code = 0x01)

int oe_cobs_stuff(uint8_t *dst, const uint8_t *src, size_t size)
{
    // Maximal payload size is is 254 bytes.
    if (size > 254)
        return -1; // OE_ECOBSPACK;

    const uint8_t *end = src + size;
    uint8_t *code_ptr = dst++; // First value is len of packet
    uint8_t code = 0x01;

    while (src < end) {
        if (*src == 0) // Encoding required
            FINISHBLOCK(code);
        else { // No encoding required
            *dst++ = *src;
            if (++code == 0xFF) //Data exceeds 254 byte len
                return -1; //OE_ECOBSPACK;
        }
        src++;
    }

    FINISHBLOCK(code);

    return 0;
}
