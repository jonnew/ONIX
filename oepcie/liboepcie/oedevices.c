#include <assert.h>

#include "oepcie.h"
#include "oedevices.h"

int oe_device_official(int dev_id)
{
    // Check if a custom device
    if (dev_id > OE_MAXDEVID)
        return 0;

    // Check if in enum
    switch (dev_id) {
        case OE_IMMEDIATEIO:
        case OE_RHD2132:
        case OE_RHD2164:
        case OE_MPU9250:
        case OE_ESTIM:
        case OE_OSTIM:
        case OE_TS4231:
            return 0;
        default:
            return OE_EDEVID;
    }
}

const char *oe_device_str(int dev_id)
{
    switch (dev_id) {
        case OE_IMMEDIATEIO: {
            return "Host-board GPIO";
        }
        case OE_RHD2132: {
            return "Intan RHD2132 bioamplifier";
        }
        case OE_RHD2164: {
            return "Intan RHD2164 bioamplifier";
        }
        case OE_MPU9250: {
            return "MPU9250 9-axis IMU";
        }
        case OE_ESTIM: {
            return "Electrical stimulation subcircuit";
        }
        case OE_OSTIM: {
            return "Optical stimulation subcircuit";
        }
        case OE_TS4231: {
            return "Triad TS4231 optical to digital converter";
        }
        default:
            return "Unknown device";
    }
}
