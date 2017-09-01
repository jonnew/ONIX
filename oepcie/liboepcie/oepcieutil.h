#ifndef OEPCIEUTIL_H
#define OEPCIEUTIL_H

#include <stdint.h>
#include <stddef.h>

int oe_cobs_stuff(uint8_t *ack, size_t length, uint8_t *buffer);
int oe_cobs_unstuff(uint8_t *ack, size_t length, uint8_t *buffer);

#endif
