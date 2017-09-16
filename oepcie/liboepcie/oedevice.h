#ifndef OEPCIE_DEVICES_H
#define OEPCIE_DEVICES_H

#include <stdint.h>

#define OE_PCIEMASTERDEVIDX     -1 // Must be negative
#define OE_READFRAMEOVERHEAD     32 // [uint64_t sample number, (24 reserved bytes), ...]
#define OE_WRITEFRAMEOVERHEAD    32 // [( 32 reserved bytes), ...]

// Supported devices/IDs
// NB: If you add a device here, make sure to update the oe_device_string array
// in oepcie.c
typedef enum oe_device_id {
    OE_MINDEVICEID                  = 0,
    OE_SERDESGPIOV1,
    OE_RHD2032,
    OE_RHD2064,
    OE_MPU9250,

    // NB: Always on bottom
    OE_MAXDEVICEID
} oe_device_id_t;

typedef enum oe_pcie_master_regs {
    OE_PCIEMASTERDEV_HEADER         = 0,
    OE_PCIEMASTERDEV_RUNNING,
    OE_PCIEMASTERDEV_ACQUIRING,
    OE_PCIEMASTERDEV_RESET,
    OE_PCIEMASTERDEV_BASECLKHZ,
    OE_PCIEMASTERDEV_FSCLKHZ,
    OE_PCIEMASTERDEV_CLKM,
    OE_PCIEMASTERDEV_CLKD,
} oe_pcie_master_regs_t;

// Fixed width device types
typedef uint32_t oe_size_t;
typedef int32_t oe_dev_idx_t;
typedef int32_t oe_dev_id_t;
typedef uint32_t oe_reg_addr_t;
typedef uint32_t oe_reg_val_t;

// TODO: The read/write types might be good targets for a tagged union so that
// sizeof can be used.
typedef struct oe_device {
    oe_dev_id_t id; // NB: Cannot use oe_device_id_t because this must be fixed width
    oe_size_t read_offset;
    oe_size_t read_size;
    oe_size_t write_offset;
    oe_size_t write_size;
} oe_device_t;

#endif
