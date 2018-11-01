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

#define OE_MAXDEVID 9999

// NB: Officially supported device IDs for the open-ephys++ project occupy
// device IDs < 10000. IDs above this value are not reserved and can be used
// for custom projects.
// NB: If you add a device here, make sure to update oe_device_str(), and
// update documentation below
typedef enum oe_device_id {
    OE_INFO              = 0, // Virtual device that provides status and error information
    OE_RHD2132           = 1, // Intan RHD2132 bioamplifier
    OE_RHD2164           = 2, // Intan RHD2162 bioamplifier
    OE_MPU9250           = 3, // MPU9250 9-axis accerometer
    OE_ESTIM             = 4, // Electrical stimulation subcircuit
    OE_OSTIM             = 5, // Optical stimulation subcircuit
    OE_TS4231            = 6, // Triad semiconductor TS421 optical to digital converter
    OE_SERDESGPO         = 7, // SERDES GPIO pins
    OE_DINPUT32          = 8, // 32-bit digital input port
    OE_DOUTPUT32         = 9, // 32-bit digital output port

    // NB: Final reserved device ID. Always on bottom
    OE_MAXDEVICEID       = OE_MAXDEVID,

    // >= 10000: Not reserved. Free to use for custom projects
} oe_devivce_id_t;


// # OE_INFO
// - Input frame data contents
//
//  [uint64_t local_clock,
//   uint32_t info_code]

enum oe_info_codes {
    OE_INFO_EWATCHDOG,          // Frame not sent withing watchdog threshold
    OE_INFO_ESERDESPARITY,     // SERDES parity error detected
    OE_INFO_ESERDESCHKSUM,     // SERDES packet checksum error detected
};

// - Configuration registers
enum oe_info_regs {
    OE_INFO_NULLPARM     = 0,  // No command
    OE_INFO_WATCHDOGEN   = 1,  // Enable frame watchdog (0 = off; 1 = 0n)
    OE_INFO_WATCHDOGDUR  = 2,  // Watchdog timer threshold (sysclock ticks, default = SYSCLOCK_HZ, 1 second)
};

// # OE_RHD2132
// - Input frame data contents
//
//  [uint64_t local_clock,
//   uint16_t chan1, uint16_t chan2, ... , uint16_t chan32,
//   uint16_t aux1, uint16_t aux2, uint16_t aux3]
//
// - Configuration registers
// TODO

// # OE_RHD2164 device
// - Input frame data contents
//
//  [uint64_t local_clock,
//   uint16_t chan1, uint16_t chan2, ... , uint16_t chan64,
//   uint16_t aux1, uint16_t aux2, uint16_t aux3]
//
// - Configuration registers
// TODO

// # OE_MPU9250
// - Input frame data contents
// TODO
//
// - Configuration registers
// TODO

// # OE_ESTIM
// - Input frame data contents: N/A
// - Configuration registers:
//      - NB: Based loosely on master-8 and pulse-pal parameters. See this page
//      for a visual definition: https://sites.google.com/site/pulsepalwiki/parameter-guide
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
    OE_ESTIM_RESET       = 16, // Reset all parameters to default
};

// # OE_OSTIM
// - Input frame data contents: N/A
// - Configuration registers
//      - NB: Based loosely on master-8 and pulse-pal parameters. See this page
//      for a visual definition: https://sites.google.com/site/pulsepalwiki/parameter-guide
enum oe_ostim_regs {
    OE_OSTIM_NULLPARM    = 0,  // No command
    OE_OSTIM_MAXCURRENT  = 2,  // Max LED/LD current, (0 to 255 = 0 to 800mA)
    OE_OSTIM_CURRENTLVL  = 3,  // Selected current level (0 to 7. Fraction of max current delivered)
    OE_OSTIM_PULSEDUR    = 5,  // Pulse duration, 10 microsecond steps
    OE_OSTIM_PULSEPERIOD = 6,  // Inter-pulse interval, 10 microsecond steps
    OE_OSTIM_BURSTCOUNT  = 7,  // Burst duration, number of pulses in burst
    OE_OSTIM_IBI         = 8,  // Inter-burst interval, microseconds
    OE_OSTIM_TRAINCOUNT  = 9,  // Pulse train duration, number of bursts in train
    OE_OSTIM_TRAINDELAY  = 10, // Pulse train delay, microseconds
    OE_OSTIM_TRIGGER     = 11, // Trigger stimulation (1 = deliver)
    OE_OSTIM_ENABLE      = 12, // Control null switch (0 = stim output shorted to ground, 1 = stim output attached to electrode during pulses)
    OE_OSTIM_RESTCURR    = 13, // Resting current between pulse phases, (0 to 255 = -1.5 mA to +1.5mA)
    OE_OSTIM_RESET       = 14, // Reset all parameters to default
};

// # OE_TS4231
// Read frame data contents:
//  [uint16_t lighthouse_id
//     uint64_t local_clock,
//   uint16_t high or low]

// # OE_SERDESGPIO
// Control GPO pins available on the DS90UB913A-Q1 serializer.
// - Input frame data contents: N/A
// - Configuration registers:
enum oe_serdesgpo_regs {
    OE_SERDESGPO_NULLPARM    = 0, // No command
    OE_SERDESGPO_NUM         = 1, // Select a GPO pin to control (0-3)
    OE_SERDESGPO_STATE       = 2, // Set the state of the selected pin (0 = LOW, other = HIGH)
    OE_SERDESGPO_RESET       = 3, // Reset all parameters to default pull all GPO pins LOW.
};

// # OE_DINPUT32
// A 32-bit digital input port. Physical device may support less digital inputs than this.
// - Input frame data contents
//
//  [uint64_t local_clock,
//   uint32_t port_state]
//
// - Configuration registers:
enum oe_dinput32_regs {
    OE_DINPUT32_NULLPARM    = 0, // No command
    OE_DINPUT32_NUM         = 1, // Select a digital input pin to control (0-31)
    OE_DINPUT32_TERM        = 2, // Toggle 50 ohm termination (0 = Off, other = On)
    OE_DINPUT32_LLEVEL      = 3, // Set logic threshold level (0-255, actual voltage depends on circuitry)
};

// # OE_DOUTPUT32
// A 32-bit digital output port. Physical device may support less digital outputs than this.
// - Input frame data contents:  N/A
// - Configuration registers:
enum oe_doutput32_regs {
    OE_DOUTPUT32_NULLPARM   = 0, // No command
    OE_DOUTPUT32_STATE      = 1, // Set the port state (32-bit unsigned integer)
    OE_DOUTPUT32_LLEVEL     = 2, // Set logic threshold level (0-255, actual voltage depends on circuitry)
};

// Human readable string from ID
OE_EXPORT const char *oe_device_str(int dev_id);

#ifdef __cplusplus
}
#endif

#endif
