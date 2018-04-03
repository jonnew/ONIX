#include <assert.h>

#include "oedevices.h"

const char *oe_device_str(int dev_id)
{
    assert(dev_id < OE_MAXDEVICEID && "Invalid device ID.");

    switch (dev_id) {
        case OE_IMMEDIATEIO: {
            return "Host-board Immediate IO";
        }
        case OE_RHD2132: {
            return "RHD2132 Bioamplifier";
        }
        case OE_RHD2164: {
            return "RHD2164 Bioamplifier";
        }
        case OE_MPU9250: {
            return "MPU9250 IMU";
        }
        case OE_ESTIM: {
            return "Headborne Electrical Stimulator";
        }
        default:
            return "Unknown device";
    }
}
