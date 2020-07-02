namespace oni.lib
{
    using System;
    using System.Runtime.InteropServices;
    using System.Security;
    using System.Text;

    // See oni_defs.h or oni_error_str()
    public enum Error
    {
        SUCCESS = 0,
        PATHINVALID = -1,
        DEVID = -2,
        DEVIDX = -3,
        WRITESIZE = -4,
        READFAILURE = -5,
        WRITEFAILURE = -6,
        NULLCTX = -7,
        SEEKFAILURE = -8,
        INVALSTATE = -9,
        INVALOPT = -10,
        INVALARG = -11,
        COBSPACK = -12,
        RETRIG = -13,
        BUFFERSIZE = -14,
        BADDEVMAP = -15,
        BADALLOC = -16,
        CLOSEFAIL = -17,
        READONLY = -18,
        UNIMPL = -19,
        INVALREADSIZE = -20,
        NOREADDEV = -21,
        INIT = -22,
        WRITEONLY = -23,
        INVALWRITESIZE = -24,
        NOTWRITEDEV = -25,
        DEVIDXREPEAT = -26,
    }

    // Make managed version of oni_device_t
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct device_t
    {
        public readonly int id;              // Device ID
        public readonly uint read_size;      // Read size
        public readonly uint num_reads;      // Num reads per sample
        public readonly uint write_size;     // Write size
        public readonly uint num_writes;     // Num writes per sample
        public readonly uint idx;            // Complete rsv.rsv.hub.idx device table index

        // Adjust this as required
        public override string ToString() =>
            $@" 0x{idx:X} : {Marshal.PtrToStringAnsi(NativeMethods.oni_device_str(id))}, Per frame read size: {read_size}, Frames per sample: {num_reads}, Write Size: {write_size}, Writes per sample: {num_writes}";

        public string Description()
        {
            return Marshal.PtrToStringAnsi(NativeMethods.oni_device_str(id));
        }
    }

    [SuppressUnmanagedCodeSecurity] // Call into native code without incurring the performance loss of a run-time security check when doing so
    public static unsafe partial class NativeMethods
    {
        public static readonly Version LibraryVersion;

        private const CallingConvention CCCdecl = CallingConvention.Cdecl;

        private const string LibraryName = "liboni";

        // The static constructor prepares static readonly fields
        static NativeMethods()
        {
            // Set once LibraryVersion to version()
            int major, minor, patch;
            oni_version(out major, out minor, out patch);
            LibraryVersion = new Version(major, minor, patch);

            // Make sure it is supported
            if (major < 2) {
                throw VersionNotSupported(null, ">= v2.0.0");
            }
        }

        private static NotSupportedException VersionNotSupported(string methodName, string requiredVersion)
        {
            return new NotSupportedException(
                    string.Format(
                        "{0}liboni version not supported. Required version {1}",
                        methodName == null ? string.Empty : methodName + ": ",
                        requiredVersion));
        }

        // liboni:
        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        private static extern void oni_version(out int major, out int minor, out int patch);

        [DllImport(LibraryName, CallingConvention = CCCdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr oni_create_ctx(string driver_name);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_init_ctx(IntPtr ctx, int host_idx);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_destroy_ctx(IntPtr ctx);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_get_opt(IntPtr ctx, int option, IntPtr val, IntPtr size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_get_opt(IntPtr ctx, int option, StringBuilder val, IntPtr size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_set_opt(IntPtr ctx, int option, IntPtr val, int size);

        [DllImport(LibraryName,  CallingConvention = CCCdecl)]
        public static extern int oni_set_opt(IntPtr ctx, int option, string val, int size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_get_driver_opt(IntPtr ctx, int option, IntPtr val, IntPtr size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_get_driver_opt(IntPtr ctx, int option, StringBuilder val, IntPtr size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_set_driver_opt(IntPtr ctx, int option, IntPtr val, int size);

        [DllImport(LibraryName,  CallingConvention = CCCdecl)]
        public static extern int oni_set_driver_opt(IntPtr ctx, int option, string val, int size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_read_reg(IntPtr ctx, uint dev_idx, uint addr, IntPtr val);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oni_write_reg(IntPtr ctx, uint dev_idx, uint addr, uint val);

        [DllImport(LibraryName, CallingConvention = CCCdecl, SetLastError = true)]
        public static extern int oni_read_frame(IntPtr ctx, out Frame frame);

        [DllImport(LibraryName, CallingConvention = CCCdecl, SetLastError = true)]
        public static extern int oni_create_frame(IntPtr ctx, out Frame frame, uint dev_idx, uint data_sz);

        [DllImport(LibraryName, CallingConvention = CCCdecl, SetLastError = true)]
        public static extern int oni_write_frame(IntPtr ctx, Frame frame);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern void oni_destroy_frame(IntPtr frame);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern IntPtr oni_error_str(int err);
    }
}
