namespace oni
{
    using System.Runtime.InteropServices;

    using lib;

    public static class Device
    {
        // Global device enumeration (see onidevices.h)
        public enum DeviceID
        {
            NULL = 0,
            INFO = 1,
            RHD2132 = 2,
            RHD2164 = 3,
            ESTIM = 4,
            OSTIM = 5,
            TS4231 = 6,
            DINPUT32 = 7,
            DOUTPUT32 = 8,
            BNO055 = 9,
            TEST = 10,
            NEUROPIX1R0 = 11,
            HEARTBEAT = 12,
            AD51X2 = 13,
            FMCVCTRL = 14,
            AD7617 = 15,
            AD576X = 16,
            TESTREG0 = 17,
            BREAKDIG1R3 = 18,
            FMCCLKIN1R3 = 19,
            FMCCLKOUT1R3 = 20,
            TS4231V2ARR = 21,
            FMCANALOG1R3 = 22,
            FMCLINKCTRL = 23,
        }

        public static string Name(int id)
        {
            return Marshal.PtrToStringAnsi(NativeMethods.oni_device_str(id));
        }
    }
}
