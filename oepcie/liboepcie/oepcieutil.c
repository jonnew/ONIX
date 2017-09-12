#include "oepcie.h"
#include "oepcieutil.h"

#define FINISHBLOCK(X) (*code_ptr = (X), code_ptr = dst++, code = 0x01)

// TODO: What if sizeof(dst) is smaller than len + 2?
int oe_cobs_stuff(const uint8_t *src, size_t len, uint8_t *dst)
{
    if (len > 254)
        return OE_ECOBSPACK;

    const uint8_t *end = src + len;
    uint8_t *code_ptr = dst++; // First value is len of packet
    uint8_t code = 0x01;

    while (src < end) {
        if (*src == 0) // Encoding required
            FINISHBLOCK(code);
        else { // No encoding required
            *dst++ = *src;
            if (++code == 0xFF) //Data exceeds 254 byte len
                return OE_ECOBSPACK;
        }
        src++;
    }

    FINISHBLOCK(code);

    return 0;
}

int oe_cobs_unstuff(const uint8_t *src, size_t len, uint8_t *dst)
{
    // Minimal COBS packet is [size, payload]. 
    // Maximal payload size is is 254 bytes.
    if (len < 2 || len > 255)
        return OE_ECOBSPACK;

    const uint8_t *end = src + len;
    while (src < end) {
        int code = *src++;
        int i; 
        for (i = 1; src < end && i < code; i++)
            *dst++ = *src++;
        if (code < 0xFF)
            *dst++ = 0;
    }

    return 0;
}
