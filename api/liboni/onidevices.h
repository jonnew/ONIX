#ifndef __ONIDEVICES_H__
#define __ONIDEVICES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define ONI_EXPORT __declspec(dllexport)
#else
#define ONI_EXPORT
#endif

#define MAXDEVID 9999

// NB: Officially supported device IDs for the open-ephys++ project occupy
// device IDs < 10,000. IDs above this value are not reserved and can be used
// for custom projects.
// NB: If you add a device here, make sure to update oni_device_str(), and
// update documentation below
typedef enum oni_device_id {
    ONI_NULL                = 0,   // Virtual device that provides status and error information
    ONI_INFO                = 1,   // Virtual device that provides status and error information
    ONI_RHD2132             = 2,   // Intan RHD2132 bioamplifier
    ONI_RHD2164             = 3,   // Intan RHD2162 bioamplifier
    ONI_ESTIM               = 4,   // Electrical stimulation subcircuit
    ONI_OSTIM               = 5,   // Optical stimulation subcircuit
    ONI_TS4231              = 6,   // Triad semiconductor TS421 optical to digital converter
    ONI_DINPUT32            = 7,   // 32-bit digital input port
    ONI_DOUTPUT32           = 8,   // 32-bit digital output port
    ONI_BNO055              = 9,   // BNO055 9-DOF IMU
    ONI_TEST0               = 10,  // A test device used for debugging
    ONI_NEUROPIX1R0         = 11,  // Neuropixels 1.0
    ONI_HEARTBEAT           = 12,  // Host heartbeat
    ONI_AD51X2              = 13,  // AD51X2 digital potentiometer
    ONI_FMCVCTRL            = 14,  // Open Ephys FMC Host Board rev. 1.3 link voltage control subcircuit
    ONI_AD7617              = 15,  // AD7617 ADC/DAS
    ONI_AD576X              = 16,  // AD576X DAC
    ONI_TESTREG0            = 17,  // A test device used for testing remote register programming
    ONI_BREAKDIG1R3         = 18,  // Open Ephys Breakout Board rev. 1.3 digital and user IO
    ONI_FMCCLKIN1R3         = 19,  // Open Ephys FMC Host Board rev. 1.3 clock input subcircuit
    ONI_FMCCLKOUT1R3        = 20,  // Open Ephys FMC Host Board rev. 1.3 clock output subcircuit
    ONI_TS4231V2ARR         = 21,  // Triad semiconductor TS421 optical to digital converter array targeting V2 base-stations
    ONI_FMCANALOG1R3        = 22,  // Open Ephys FMC Host Board rev. 1.3 analog IO subcircuit
    ONI_FMCLINKCTRL         = 23,  // Open Ephys FMC Host Board coaxial headstage link control circuit

    // NB: Final reserved device ID. Always on bottom
    ONI_MAXDEVICEID          = MAXDEVID,

    // >= 10000: Not reserved. Free to use for custom projects
} oni_devivce_id_t;

// # ONI_INFO
// - Input frame data contents: [uint64_t local_clock,
//                               uint32_t info_code]
enum oni_info_codes {
    ONI_INFO_ELOSTREMOTE     = 0,   // Lost remote lock
    ONI_INFO_EBADPACKET      = 1,   // Malformed packet during SERDES demultiplexing
    ONI_INFO_EREMOTEINIT     = 2,   // Remote initialization error
    ONI_INFO_ETOOMANYREMOTE  = 3,   // TOO many remote devices for host to support
    ONI_INFO_ESERDESCHKSUM   = 4,   // SERDES packet checksum error detected
    ONI_INFO_ESERDESPARITY   = 5,   // SERDES parity error detected
    ONI_INFO_EWATCHDOG       = 6,   // Frame not sent withing watchdog threshold
};
// - Output data: N/A
// - Configuration registers: N/A

// - Configuration registers
enum oni_info_regs {
    ONI_INFO_NULLPARM        = 0,   // No command
    ONI_INFO_WATCHDOGEN      = 1,   // Enable frame watchdog (0 = off; 1 = 0n)
    ONI_INFO_WATCHDOGDUR     = 2,   // Watchdog timer threshold (sysclock ticks, default = SYSCLOCK_HZ, 1 second)
};

// # ONI_RHD2132
// - Input frame data contents: [uint64_t local_clock,
//                               uint16_t chan1, uint16_t chan2, ... , uint16_t chan32,
//                               uint16_t aux1, uint16_t aux2, uint16_t aux3]
// - Output data: N/A
// - Configuration registers: See the RHD2132 datasheet (http://intantech.com/files/Intan_RHD2132_datasheet.pdf)

// # ONI_RHD2164 device
// - Input frame data contents: [uint64_t local_clock,
//                               uint16_t chan1, uint16_t chan2, ... , uint16_t chan64,
//                               uint16_t aux1, uint16_t aux2, uint16_t aux3]
// - Output data: N/A
// - Configuration registers: See the RHD2164 datasheet (http://intantech.com/files/Intan_RHD2164_datasheet.pdf)

// # ONI_ESTIM
// Electrical stimulation subcircuit
// - Input frame data contents: N/A
// - Output data: N/A
// - Configuration registers: Based loosely on master-8 and pulse-pal parameters. See this page
//                            for a visual definition: https://sites.google.com/site/pulsepalwiki/parameter-guide
enum oni_estim_regs {
    ONI_ESTIM_NULLPARM       = 0,   // No command
    ONI_ESTIM_BIPHASIC       = 1,   // Biphasic pulse (0 = monophasic, 1 = biphasic; NB: currently ignored)
    ONI_ESTIM_CURRENT1       = 2,   // Phase 1 current, (0 to 255 = -1.5 mA to +1.5mA)
    ONI_ESTIM_CURRENT2       = 3,   // Phase 2 voltage, (0 to 255 = -1.5 mA to +1.5mA)
    ONI_ESTIM_PULSEDUR1      = 4,   // Phase 1 duration, microseconds
    ONI_ESTIM_IPI            = 5,   // Inter-phase interval, microseconds
    ONI_ESTIM_PULSEDUR2      = 6,   // Phase 2 duration, microseconds
    ONI_ESTIM_PULSEPERIOD    = 7,   // Inter-pulse interval, microseconds
    ONI_ESTIM_BURSTCOUNT     = 8,   // Burst duration, number of pulses in burst
    ONI_ESTIM_IBI            = 9,   // Inter-burst interval, microseconds
    ONI_ESTIM_TRAINCOUNT     = 10,  // Pulse train duration, number of bursts in train
    ONI_ESTIM_TRAINDELAY     = 11,  // Pulse train delay, microseconds
    ONI_ESTIM_TRIGGER        = 12,  // Trigger stimulation (1 = deliver)
    ONI_ESTIM_POWERON        = 13,  // Control estim sub-circuit power (0 = off, 1 = on)
    ONI_ESTIM_ENABLE         = 14,  // Control null switch (0 = stim output shorted to ground, 1 = stim output attached to electrode during pulses)
    ONI_ESTIM_RESTCURR       = 15,  // Resting current between pulse phases, (0 to 255 = -1.5 mA to +1.5mA)
    ONI_ESTIM_RESET          = 16,  // Reset all parameters to default
};

// # ONI_OSTIM
// Optical stimulation subcircuit
// - Input frame data contents: N/A
// - Output data: N/A
// - Configuration registers: Based loosely on master-8 and pulse-pal parameters. See this page
//                            for a visual definition: https://sites.google.com/site/pulsepalwiki/parameter-guide
enum oni_ostim_regs {
    ONI_OSTIM_NULLPARM       = 0,   // No command
    ONI_OSTIM_MAXCURRENT     = 1,   // Max LED/LD current, (0 to 255 = 0 to 800mA)
    ONI_OSTIM_CURRENTLVL     = 2,   // Selected current level (0 to 7. Fraction of max current delivered)
    ONI_OSTIM_PULSEDUR       = 3,   // Pulse duration, microseconds
    ONI_OSTIM_PULSEPERIOD    = 4,   // Inter-pulse interval, microseconds
    ONI_OSTIM_BURSTCOUNT     = 5,   // Burst duration, number of pulses in burst
    ONI_OSTIM_IBI            = 6,   // Inter-burst interval, microseconds
    ONI_OSTIM_TRAINCOUNT     = 7,   // Pulse train duration, number of bursts in train (0 = continuous)
    ONI_OSTIM_TRAINDELAY     = 8,   // Pulse train delay, microseconds
    ONI_OSTIM_TRIGGER        = 9,   // Trigger stimulation (1 = deliver)
    ONI_OSTIM_ENABLE         = 10,  // Control null switch (0 = stim output shorted to ground, 1 = stim output attached to electrode during pulses)
    ONI_OSTIM_RESTCURR       = 11,  // Resting current between pulse phases, (0 to 7. Fraction of max current)
    ONI_OSTIM_RESET          = 12,  // Reset all parameters to default
};

// # ONI_TS4231
// HTC vive lighthouse receiver
// - Input frame data contents: [uint16_t lighthouse_id
//                               uint64_t local_clock,
//                               uint16_t high or low]
// - Output data: N/A
// - Configuration registers: N/A

// # ONI_DINPUT32
// A 32-bit digital input port. Physical device may support less digital inputs than this.
// - Input frame data contents : [uint64_t local_clock,
//                                uint32_t port_state]
// - Output data: N/A
// - Configuration registers:
enum oni_dinput32_regs {
    ONI_DINPUT32_NULLPARM    = 0,   // No command
    ONI_DINPUT32_NUM         = 1,   // Select a digital input pin to control (0-31)
    ONI_DINPUT32_TERM        = 2,   // Toggle 50 ohm termination (0 = Off, other = On)
    ONI_DINPUT32_LLEVEL      = 3,   // Set logic threshold level (0-255, actual voltage depends on circuitry)
};

// # ONI_DOUTPUT32
// A 32-bit digital output port. Physical device may support less digital outputs than this.
// - Input frame data contents:  N/A
// - Output data: [uint32_t port_state]
// - Configuration registers:
enum oni_doutput32_regs {
    ONI_DOUTPUT32_NULLPARM   = 0,   // No command
    ONI_DOUTPUT32_LLEVEL     = 1,   // Set logic threshold level (0-255, actual voltage depends on circuitry)
};


// # ONI_BNO055
// - Input frame data contents
// TODO
//
// - Configuration registers
// TODO

// # ONI_TEST0
// - Input frame data contents
// TODO
//
// - Configuration registers
// TODO

// # ONI_NEUROPIX1R0
// IMEC Neuropixels revision 1.0
// - Input frame data contents: [uint64_t local_clock,
//                               uint16_t frame_type,
//                               uint16_t[40] frame_data]
// - Output data: N/A
// - Configuration registers: UNDOCUMENTED

// # ONI_HEARTBEAT
// Heartbeat device
// - Input frame data contents: [uint64_t local_clock]
// - Output data: N/A
// - Configuration registers: N/A

// # ONI_AD51X2_ID
// Digital Potentiometer
// - Input frame data contents: N/A
// - Output data: N/A
// - Configuration registers: See the AD51X2 datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/AD5122_5142.pdf

// # ONI_FMCVCTRL_ID
// Open Ephys FMC Host Board rev. 1.3 link voltage controller
// - Input frame data contents: N/A
// - Output data: N/A
// - Configuration registers
enum oni_fmcvtrl_regs {
    ONI_FMCVCTRL_NULLPARM = 0, // No command
    ONI_FMCVCTRL_LINK = 1, // Select a link to control (0 to 1) value set to 10*V_desired (e.g. if you want 3.3V, value is 33).
};

// Human readable string from ID
ONI_EXPORT const char *oni_device_str(int dev_id);

#ifdef __cplusplus
}
#endif

#endif
