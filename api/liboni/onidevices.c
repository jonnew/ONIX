#include <assert.h>

#include "oni.h"
#include "onidevices.h"

const char *oni_device_str(int dev_id)
{
    switch (dev_id) {
        case ONI_NULL: {
            return "Placeholder device. Neither generates or accepts data.";
        }
        case ONI_INFO: {
            return "Host status and error information.";
        }
        case ONI_RHD2132: {
            return "Intan RHD2132 bioamplifier";
        }
        case ONI_RHD2164: {
            return "Intan RHD2164 bioamplifier";
        }
        case ONI_MPU9250: {
            return "MPU9250 9-axis IMU";
        }
        case ONI_ESTIM: {
            return "Electrical stimulation subcircuit";
        }
        case ONI_OSTIM: {
            return "Optical stimulation subcircuit";
        }
        case ONI_TS4231: {
            return "Triad TS4231 optical to digital converter";
        }
        case ONI_SERDESGPO: {
            return "GPO pins available on the DS90UB913A-Q1 serializer";
        }
        case ONI_DINPUT32: {
            return "32-bit digital input port";
        }
        case ONI_DOUTPUT32: {
            return "32-bit digital output port";
        }
        case ONI_BNO055: {
            return "BNO055 9-axis IMU";
        }
        case ONI_TEST0: {
            return "Open Ephys test device";
        }
        case ONI_NEUROPIX1R0: {
            return "Neuropixels 1.0 probe";
        }
        case ONI_HEARTBEAT: {
            return "Host heartbeat";
        }
        default:
            return "Unknown device";
    }
}
