namespace oe.lib
{
    using System;
    using System.Runtime.InteropServices;

    public static class oedevices
    {
        private const CallingConvention CCCdecl = CallingConvention.Cdecl;

        private const string LibraryName = "liboepcie";

        [DllImport(LibraryName, EntryPoint = "oe_device_str", CallingConvention = CCCdecl)]
        public static extern IntPtr device_str(Int32 id);
    }
}
