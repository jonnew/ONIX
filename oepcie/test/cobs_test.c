#include "assert.h"
#include "errno.h"
#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#include "oepcie.h"

// Signal flags
typedef enum oe_signal {
    NULLSIG             = (1u << 0),
    CONFIGWACK          = (1u << 1), // Configuration write-acknowledgement
    CONFIGWNACK         = (1u << 2), // Configuration no-write-acknowledgement
    CONFIGRACK          = (1u << 3), // Configuration read-acknowledgement
    CONFIGRNACK         = (1u << 4), // Configuration no-read-acknowledgement
    DEVICEMAPACK        = (1u << 5), // Device map start acnknowledgement
    FRAMERSIZE          = (1u << 6), // Frame read size in bytes
    FRAMEWSIZE          = (1u << 7), // Frame write size in bytes
    DEVICEINST          = (1u << 8), // Deivce map instance
} oe_signal_t;

static inline int _oe_read(int data_fd, void *data, size_t size)
{
    size_t received = 0;

    while (received < size) {

        int rc = read(data_fd, (char *)data + received, size - received);

        if ((rc < 0) && (errno == EINTR))
            continue;

        if (rc <= 0)
            return OE_EREADFAILURE;

        received += rc;
    }

    return received;
}

static inline int _oe_read_signal_packet(int signal_fd, uint8_t *buffer)
{
    // Read the next zero-deliminated packet
    int i = 0;
    uint8_t curr_byte = 1;
    int bad_delim = 0;
    while (curr_byte != 0) {
        int rc = _oe_read(signal_fd, &curr_byte, 1);
        if (rc != 1) return rc;

        if (i < 255)
            buffer[i] = curr_byte;
        else
            bad_delim = 1;

        i++;
    }

    if (bad_delim)
        return OE_ECOBSPACK;
    else
        return --i; // Length of packet without 0 delimeter
}

static int _oe_cobs_unstuff(uint8_t *dst, const uint8_t *src, size_t size)
{
    // Minimal COBS packet is 1 overhead byte + 1 data byte
    // Maximal COBS packet is 1 overhead byte + 254 datate bytes
    assert(size >= 2 && size <= 255 && "Invalid COBS packet buffer size.");

    const uint8_t *end = src + size;
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

int main ( int argc, char *argv[] ) {

    if (argc != 2) /* argc should be 2 for correct execution */
    {
        /* We print argv[0] assuming it is the program name */
        printf("usage: %s signal-path\n", argv[0]);
        return -1;
    }

    oe_signal_t packet_type = NULLSIG;
    uint8_t buffer[255] = {0};
    int signal_fd = open(argv[1], O_RDONLY);
    if (signal_fd == -1) {
        printf("Could not open signal device file.\n");
        return -1;
    }

    while(1) {
        int pack_size = _oe_read_signal_packet(signal_fd, buffer);

        if (pack_size < 1) {
            printf("Something wrong with delimiter, try again.");
            continue;
        }

        // Unstuff the packet (last byte is the 0, so we decrement
        int rc = _oe_cobs_unstuff(buffer, buffer, pack_size);
        if (rc < 0) {
            printf("Something wrong with packet, try again.");
            continue; // Something wrong with packet, try again
        }

        // Get the type, which occupies first 4 bytes of buffer
        packet_type = *(oe_signal_t *)buffer;

        printf("Received packet:\n");
        printf("   Type:%u", (uint32_t)packet_type);
        printf("   Payload:");
        for (int i = 4; i < pack_size - 1; i++) { // NB: -1 for overhead byte
            printf("%x ", *(buffer+i));
        }

        printf("\n");
    } 

    return 0;
}
