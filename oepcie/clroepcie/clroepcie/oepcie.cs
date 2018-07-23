namespace oe.lib
{
    using System;
    using System.Runtime.InteropServices;
    using System.Security;
    using System.Text;

    public enum Error
    {
        SUCCESS = 0,  // Success
        PATHINVALID = -1,  // Invalid stream path, fail on open
        REINITCTX = -2,  // Double initialization attempt
        DEVID = -3,  // Invalid device ID on init or reg op
        READFAILURE = -4,  // Failure to read from a stream/register
        WRITEFAILURE = -5,  // Failure to write to a stream/register
        NULLCTX = -6,  // Attempt to call function with null ctx
        SEEKFAILURE = -7,  // Failure to seek on stream
        INVALSTATE = -8,  // Invalid operation for the current context run state
        DEVIDX = -9,  // Invalid device index
        INVALOPT = -10, // Invalid context option
        INVALARG = -11, // Invalid function arguments
        CANTSETOPT = -12, // Option cannot be set in current context state
        COBSPACK = -13, // Invalid COBS packet
        RETRIG = -14, // Attempt to trigger an already triggered operation
        BUFFERSIZE = -15, // Supplied buffer is too small
        BADDEVMAP = -16, // Badly formated device map supplied by firmware
        BADALLOC = -17, // Bad dynamic memory allocation
        CLOSEFAIL = -18, // File descriptor close failure, check errno
        DATATYPE = -19, // Invalid underlying data types
        READONLY = -20, // Attempted write to read only object (register, context option, etc)
        RUNSTATESYNC = -21, // Software and hardware run state out of sync
        INVALRAWTYPE = -22, // Invalid raw data type
    }

    // Make managed version of oe_device_t
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct device_t
    {
        public uint id;             // Device ID
        public uint read_size;      // Read size
        public uint num_reads;      // Num reads per sample
        public uint write_size;     // Write size
        public uint num_writes;     // Num writes per sample
    }

    [SuppressUnmanagedCodeSecurity] // Call into native code without incurring the performance loss of a run-time security check when doing so
    public static unsafe partial class NativeMethods
    {
        public static readonly Version LibraryVersion;

        private const CallingConvention CCCdecl = CallingConvention.Cdecl;

        private const string LibraryName = "liboepcie";
        public const string DefaultConfigPath = "\\\\.\\xillybus_cmd_mem_32";
        public const string DefaultReadPath = "\\\\.\\xillybus_data_read_32";
        public const string DefaultSignalPath = "\\\\.\\xillybus_async_read_8";

        // The static constructor prepares static readonly fields
        static NativeMethods()
        {
            // Set once LibraryVersion to version()
            int major, minor, patch;
            oe_version(out major, out minor, out patch);
            LibraryVersion = new Version(major, minor, patch);

            // Make sure it is supported
            if (major < 1) {
                throw VersionNotSupported(null, ">= v1.0.0");
            }
        }

        private static NotSupportedException VersionNotSupported(string methodName, string requiredVersion)
        {
            return new NotSupportedException(
                    string.Format(
                        "{0}liboepcie version not supported. Required version {1}",
                        methodName == null ? string.Empty : methodName + ": ",
                        requiredVersion));
        }

        // liboepcie:

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        private static extern void oe_version(out int major, out int minor, out int patch);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern IntPtr oe_create_ctx();

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oe_init_ctx(IntPtr ctx);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oe_destroy_ctx(IntPtr ctx);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oe_get_opt(IntPtr ctx, int option, IntPtr val, IntPtr size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oe_get_opt(IntPtr ctx, int option, StringBuilder val, IntPtr size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oe_set_opt(IntPtr ctx, int option, IntPtr val, int size);

        [DllImport(LibraryName, CharSet= CharSet.Ansi, CallingConvention = CCCdecl)]
        public static extern int oe_set_opt(IntPtr ctx, int option, string val, int size);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oe_read_reg(IntPtr ctx, uint dev_idx, uint addr, IntPtr val);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern int oe_write_reg(IntPtr ctx, uint dev_idx, uint addr, uint val);

        [DllImport(LibraryName, CallingConvention = CCCdecl, SetLastError = true)]
        public static extern int oe_read_frame(IntPtr ctx, out Frame frame);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern void oe_destroy_frame(IntPtr frame);

        [DllImport(LibraryName, CallingConvention = CCCdecl)]
        public static extern IntPtr oe_error_str(int err);
    }
}
