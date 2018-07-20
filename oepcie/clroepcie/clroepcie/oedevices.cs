namespace oe.lib
{
    using System;
    using System.Runtime.InteropServices;

    public static partial class NativeMethods
    {
        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern IntPtr oe_device_str(int id);
    }
}
