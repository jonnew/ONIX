using System.Runtime.InteropServices;

namespace oe
{
    using lib;
    
    public static class Device
    {
        public static string Name(int id)
        {
            return Marshal.PtrToStringAnsi(oedevices.device_str(id));
        }

        public enum DeviceID
        {
            IMMEDIATEIO = 0,
            RHD2132 = 1,
            RHD2164 = 2,
            MPU9250 = 3,
            ESTIM = 4,
            OSTIM = 5,
            TS4231 = 6
        }

        // ** IMMEDIATEIO device configuration registers **
        // TODO

        // ** RHD2132 device configuration registers **
        // TODO

        // ** RHD2164 device configuration registers **
        // TODO

        // ** MPU9250 device configuration registers **
        // TODO

        // ** EstimRegister device configuration registers **
        // NB: Based loosely on master-8 and pulse-pal parameters. See this page for a
        // visual definition:
        // https://sites.google.com/site/pulsepalwiki/parameter-guide
        public enum EstimRegister
        {
            NULLPARM = 0,  // No command
            BIPHASIC = 1,  // Biphasic pulse (0 = monophasic, 1 = biphasic; NB: currently ignored)
            CURRENT1 = 2,  // Phase 1 current, (0 to 255 = -1.5 mA to +1.5mA)
            CURRENT2 = 3,  // Phase 2 voltage, (0 to 255 = -1.5 mA to +1.5mA)
            PULSEDUR1 = 4,  // Phase 1 duration, 10 microsecond steps
            IPI = 5,  // Inter-phase interval, 10 microsecond steps
            PULSEDUR2 = 6,  // Phase 2 duration, 10 microsecond steps
            PULSEPERIOD = 7,  // Inter-pulse interval, 10 microsecond steps
            BURSTCOUNT = 8,  // Burst duration, number of pulses in burst
            IBI = 9,  // Inter-burst interval, microseconds
            TRAINCOUNT = 10, // Pulse train duration, number of bursts in train
            TRAINDELAY = 11, // Pulse train delay, microseconds
            TRIGGER = 12, // Trigger stimulation (1 = deliver)
            POWERON = 13, // Control estim sub-circuit power (0 = off, 1 = on)
            ENABLE = 14, // Control null switch (0 = stim output shorted to ground, 1 = stim output attached to electrode during pulses)
            RESTCURR = 15, // Resting current between pulse phases, (0 to 255 = -1.5 mA to +1.5mA)
            RESET = 16, // Reset all parameters to default
        }
    }
}
