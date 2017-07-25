#ifndef OEPCIE_DEVICES_H
#define OEPCIE_DEVICES_H

// Supported devices/IDs
#define RHD2032              0
#define RHD2064              1
#define MPU950               2

// NB: Changed each time a device is added
#define MAX_DEVICE_ID        MPU950
#define NUM_DEVICE_TYPES     3

typedef struct device {
    size_t read_offset;
    size_t read_size;
    size_t write_offset;
    size_t write_size;
    int id;
} device_t;

// Static device map
// Global array of devices structs with hardcoded read, write, and ID
// information for every device supported by this library.  Device ID numbers
// obtained from the header stream is used to index into this array.
static const device_t devices[NUM_DEVICE_TYPES] = {
    {.read_offset = 0, .read_size = (16 * (32 + 3)),
        .write_offset = 0, .write_size = 0, .id = RHD2032},
    {.read_offset = 0, .read_size = (16 * (64 + 3)),
        .write_offset = 0, .write_size = 0, .id = RHD2064},
    {.read_offset = 0, .read_size = (32 * 6),
        .write_offset = 0, .write_size = 0, .id = MPU950}
};

#endif
