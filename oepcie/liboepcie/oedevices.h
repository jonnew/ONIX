#ifndef __OEDEVICES_H__
#define __OEDEVICES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define OE_EXPORT __declspec(dllexport)
#else
#define OE_EXPORT
#endif

// Supported devices/IDs for the open-ephys++ project. oedevices.* can be
// replaced with any device set if a user wishes to redefine these.
// NB: If you add a device here, make sure to update oe_device_str() and add
// registers below
enum oe_device_id {
    OE_IMMEDIATEIO = 0,
    OE_RHD2132	   = 1,
    OE_RHD2164     = 2,
    OE_MPU9250     = 3,
    OE_ESTIM       = 4,
    OE_OSTIM       = 5,
    OE_PCECAMV3    = 6,

    // NB: Always on bottom
    OE_MAXDEVICEID
};

// ** OE_IMMEDIATEIO device configuration registers **
// TODO

// ** OE_RHD2132 device configuration registers **
// TODO

// ** OE_RHD2164 device configuration registers **
// TODO

// ** OE_MPU9250 device configuration registers **
// TODO

// ** OE_ESTIM device configuration registers **
// NB: Based loosely on master-8 and pulse-pal parameters. See this page for a
// visual definition:
// https://sites.google.com/site/pulsepalwiki/parameter-guide

enum oe_estim_regs {
    OE_ESTIM_NULLPARM    = 0,  // No command
    OE_ESTIM_BIPHASIC    = 1,  // Biphasic pulse (0 = monophasic, 1 = biphasic; NB: currently ignored)
    OE_ESTIM_CURRENT1    = 2,  // Phase 1 current, (0 to 255 = -1.5 mA to +1.5mA)
    OE_ESTIM_CURRENT2    = 3,  // Phase 2 voltage, (0 to 255 = -1.5 mA to +1.5mA)
    OE_ESTIM_PULSEDUR1   = 4,  // Phase 1 duration, 10 microsecond steps
    OE_ESTIM_IPI         = 5,  // Inter-phase interval, 10 microsecond steps
    OE_ESTIM_PULSEDUR2   = 6,  // Phase 2 duration, 10 microsecond steps
    OE_ESTIM_PULSEPERIOD = 7,  // Inter-pulse interval, 10 microsecond steps
    OE_ESTIM_BURSTCOUNT  = 8,  // Burst duration, number of pulses in burst
    OE_ESTIM_IBI         = 9,  // Inter-burst interval, microseconds
    OE_ESTIM_TRAINCOUNT  = 10, // Pulse train duration, number of bursts in train
    OE_ESTIM_TRAINDELAY  = 11, // Pulse train delay, microseconds
    OE_ESTIM_TRIGGER     = 12, // Trigger stimulation (1 = deliver)
    OE_ESTIM_POWERON     = 13, // Control estim sub-circuit power (0 = off, 1 = on)
    OE_ESTIM_ENABLE      = 14, // Control null switch (0 = stim output shorted to ground, 1 = stim output attached to electrode during pulses)
    OE_ESTIM_RESTCURR    = 15, // Resting current between pulse phases, (0 to 255 = -1.5 mA to +1.5mA)
	OE_ESTIM_RESET		 = 16, // Reset all parameters to default
};

// ** OE_OSTIM device configuration registers **
// TODO

// Human readable string from ID
OE_EXPORT const char *oe_device_str(int dev_id);

#ifdef __cplusplus
}
#endif


#endif
