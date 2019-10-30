namespace oe
{
    using System.Runtime.InteropServices;

    using lib;
    
    public static class Device
    {
        // Global device enumeration (see oedevices.h)
        public enum DeviceID
        {
            NULL = 0,
            INFO = 1,
            RHD2132 = 2,
            RHD2164 = 3,
            MPU9250 = 4,
            ESTIM = 5,
            OSTIM = 6,
            TS4231 = 7,
            SERDESGPO = 8,
            DINPUT32 = 9,
            DOUTPUT32 = 10,
            BNO055 = 11,
            TEST = 12,
            NEUROPIX1R0 = 13,
            HEARTBEAT = 14
        }

        public static string Name(int id)
        {
            return Marshal.PtrToStringAnsi(NativeMethods.oe_device_str(id));
        }

        // TODO: Everything below needs to be moved to appropriate device class once it exists


        // TODO: Move to OStim device
        // public enum OstimRegsiter
        //{
        //    OSTIM_NULLPARM = 0,  // No command
        //    OSTIM_MAXCURRENT = 2,  // Max LED/LD current, (0 to 255 = 0 to 800mA)
        //    OSTIM_CURRENTLVL = 3,  // Selected current level (0 to 7. Fraction of max current delivered)
        //    OSTIM_PULSEDUR = 5,  // Pulse duration, 10 microsecond steps
        //    OSTIM_PULSEPERIOD = 6,  // Inter-pulse interval, 10 microsecond steps
        //    OSTIM_BURSTCOUNT = 7,  // Burst duration, number of pulses in burst
        //    OSTIM_IBI = 8,  // Inter-burst interval, microseconds
        //    OSTIM_TRAINCOUNT = 9,  // Pulse train duration, number of bursts in train
        //    OSTIM_TRAINDELAY = 10, // Pulse train delay, microseconds
        //    OSTIM_TRIGGER = 11, // Trigger stimulation (1 = deliver)
        //    OSTIM_ENABLE = 12, // Control null switch (0 = stim output shorted to ground, 1 = stim output attached to electrode during pulses)
        //    OSTIM_RESTCURR = 13, // Resting current between pulse phases, (0 to 255 = -1.5 mA to +1.5mA)
        //    OSTIM_RESET = 14, // Reset all parameters to default
        //}

        // TODO: Move to SerdesGPO
        //public enum SerdesGPORegister
        //{
        //    SERDESGPO_NULLPARM = 0, // No command
        //    SERDESGPO_NUM = 1, // Select a GPO pin to control (0-3)
        //    SERDESGPO_STATE = 2, // Set the state of the selected pin (0 = LOW, other = HIGH)
        //    SERDESGPO_RESET = 3, // Reset all parameters to default pull all GPO pins LOW.
        //}



        // TODO: Move to DigitalOutput32
        //enum DigitalOutput32Register
        //{
        //    DOUTPUT32_NULLPARM = 0, // No command
        //    DOUTPUT32_STATE = 1, // Set the port state (32-bit unsigned integer)
        //    DOUTPUT32_LLEVEL = 2, // Set logic threshold level (0-255, actual voltage depends on circuitry)
        //};
    }
}
