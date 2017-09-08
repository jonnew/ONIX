#ifndef OEPCIEUTIL_H
#define OEPCIEUTIL_H

#include <stddef.h>
#include <stdint.h>

int oe_cobs_stuff(const uint8_t *src, size_t length, uint8_t *buffer);
int oe_cobs_unstuff(const uint8_t *src, size_t length, uint8_t *buffer);

#endif
