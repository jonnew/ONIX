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
        case ONI_ESTIM: {
            return "Electrical stimulation subcircuit";
        }
        case ONI_OSTIM: {
            return "Optical stimulation subcircuit";
        }
        case ONI_TS4231: {
            return "Triad TS4231 optical to digital converter";
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
        case ONI_AD51X2: {
            return "AD51X2 digital potentiometer";
        }
        case ONI_FMCVCTRL: {
            return "Open Ephys FMC Host Board rev. 1.3 link voltage control subcircuit";
        }
        case ONI_AD7617: {
            return "AD7617 ADC/DAS";
        }
        case ONI_AD576X: {
            return "AD576x DAC";
        }
        case ONI_TESTREG0: {
            return "A test device used for testing remote register programming";
        }
        case ONI_BREAKDIG1R3: {
            return "Open Ephys Breakout board rev. 1.3 digital and user IO";
        }
        case ONI_FMCCLKIN1R4: {
            return "Open Ephys FMC Host Board rev. 1.4 clock intput subcircuit";
        }
        case ONI_FMCCLKOUT1R4: {
            return "Open Ephys FMC Host Board rev. 1.4 clock output subcircuit";
        }
        default:
            return "Unknown device";
    }
}

