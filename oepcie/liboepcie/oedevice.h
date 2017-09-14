#ifndef OEPCIE_DEVICES_H
#define OEPCIE_DEVICES_H

#include <stdint.h>

// NB: Changed each time a device is added
#define OEPCIEMASTER        -1 // Must be negative
#define MIN_DEVICE_ID        OESERDESGPIOV1
#define MAX_DEVICE_ID        MPU950

// Supported devices/IDs
typedef enum oe_device_id {
    OESERDESGPIOV1    = 1,
    RHD2032           = 100,
    RHD2064           = 101,
    MPU950            = 102
} oe_device_id_t;

typedef enum oe_pcie_master_regs {
    OEPCIEMASTER_HEADER = 0,
    OEPCIEMASTER_RUNNING = 1
} oe_pcie_master_regs_t;

typedef int32_t oe_id_t;
typedef uint32_t oe_size_t;

typedef struct oe_device {
    oe_id_t id; // Cannot use oe_device_id_t because this must be fixed width
    oe_size_t read_offset;
    oe_size_t read_size;
    oe_size_t write_offset;
    oe_size_t write_size;
} oe_device_t;

#endif
