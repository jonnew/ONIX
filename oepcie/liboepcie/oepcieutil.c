#include "oepcie.h"
#include "oepcieutil.h"

#define FinishBlock(X) (*code_ptr = (X), code_ptr = buffer++, code = 0x01)

// TODO: Set ack appropriately
int oe_cobs_stuff(uint8_t *ack, size_t length, uint8_t *buffer)
{
    if (length > 254)
        return OE_ECOBSPACK;

    const uint8_t *end = ack + length;
    uint8_t *code_ptr = buffer++;
    uint8_t code = 0x01;

    while (ack < end) {
        if (*ack == 0)
            FinishBlock(code);
        else {
            *buffer++ = *ack;
            if (++code == 0xFF)
                FinishBlock(code);
        }
        ack++;
    }

    FinishBlock(code);

    return 0;
}

int oe_cobs_unstuff(uint8_t *ack, size_t length, uint8_t *buffer)
{
    if (length > 254)
        return OE_ECOBSPACK;

    const uint8_t *end = ack + length;
    while (ack < end) {
        int code = *ack++;
        for (int i = 1; ack < end && i < code; i++)
            *buffer++ = *ack++;
        if (code < 0xFF)
            *buffer++ = 0;
    }

    return 0;
}
