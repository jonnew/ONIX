namespace oe.lib
{
    using System;
    using System.Runtime.InteropServices;

    public static unsafe class oepcie
    {
        public static readonly Version LibraryVersion;

        private const CallingConvention CCCdecl = CallingConvention.Cdecl;

        private const string LibraryName = "liboepcie";

        public const string DefaultConfigPath = "\\\\.\\xillybus_cmd_mem_32";
        public const string DefaultReadPath = "\\\\.\\xillybus_data_read_32";
        public const string DefaultSignalPath = "\\\\.\\xillybus_async_read_8";

        public enum Error
        {
            SUCCESS = 0,  // Success
            PATHINVALID = -1,  // Invalid stream path, fail on open
            REINITCTX = -2,  // Double initialization attempt
            DEVID = -3,  // Invalid device ID on init or reg op
            READFAILURE = -4,  // Failure to read from a stream/register
            WRITEFAILURE = -5,  // Failure to write to a stream/register
            NULLCTX = -6,  // Attempt to call function w null ctx
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

        //public enum DeviceID
        //{
        //    IMMEDIATEIO = 0,
        //    RHD2132 = 1,
        //    RHD2164 = 2,
        //    MPU9250 = 3,
        //    ESTIM = 4,
        //}

        // Make managed version of oe_frame_t
        [StructLayout(LayoutKind.Sequential)]
        public struct frame_t
        {
            //[MarshalAs(UnmanagedType.U8)]
            public ulong clock;       // Base clock counter
            //[MarshalAs(UnmanagedType.U2)]
            public ushort num_dev;     // Number of devices in frame
            //[MarshalAs(UnmanagedType.U1)]
            public byte corrupt;       // Is this frame corrupt?
            public uint *dev_idxs;   // Array of device indices in frame
            //[MarshalAs(UnmanagedType.U4)]
            public uint dev_idxs_sz; // Size in bytes of dev_idxs buffer
            public uint *dev_offs;   // Device data offsets within data block
            //[MarshalAs(UnmanagedType.U4)]
            public uint dev_offs_sz; // Size in bytes of dev_idxs buffer
            public byte *data;         // Multi-device raw data block
            //[MarshalAs(UnmanagedType.U4)]
            public uint data_sz;     // Size in bytes of data buffer
        }

        // Make managed version of oe_device_t
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct device_t
        {
            //[MarshalAs(UnmanagedType.U4)]
            public uint id;             // Device ID
            //[MarshalAs(UnmanagedType.U4)]
            public uint read_size;      // Read size
            //[MarshalAs(UnmanagedType.U4)]
            public uint num_reads;      // Num reads per sample
            //[MarshalAs(UnmanagedType.U4)]
            public uint write_size;     // Write size
            //[MarshalAs(UnmanagedType.U4)]
            public uint num_writes;     // Num writes per sample

        }

        // The static constructor prepares static readonly fields
        static oepcie()
        {
            // Set once LibraryVersion to version()
            int major, minor, patch;
            version(out major, out minor, out patch);
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

        // (1) Declare privately the extern entry point
        [DllImport(LibraryName, EntryPoint = "oe_version", CallingConvention = CCCdecl)]
        private static extern void version(out int major, out int minor, out int patch);

        // (2) Describe the extern function using a delegate
        //private delegate void oe_version_delegate(out int major, out int minor, out int patch);

        // (3) Save and return the managed delegate to the unmanaged function
        //     This static readonly field definition allows to be
        //     initialized and possibly redirected by the static constructor.
        //     (usually public, but we can access this through LibraryVersion)
        //private static readonly oe_version_delegate version = oe_version;

        // Repeat for the rest of the API
        [DllImport(LibraryName, EntryPoint = "oe_create_ctx", CallingConvention = CCCdecl)]
        public static extern IntPtr create_ctx();

        [DllImport(LibraryName, EntryPoint = "oe_init_ctx", CallingConvention = CCCdecl)]
        public static extern Int32 init_ctx(IntPtr ctx);

        [DllImport(LibraryName, EntryPoint = "oe_destroy_ctx", CallingConvention = CCCdecl)]
        public static extern Int32 destroy_ctx(IntPtr ctx);

        [DllImport(LibraryName, EntryPoint = "oe_get_opt", CallingConvention = CCCdecl)]
        public static extern Int32 get_opt(IntPtr ctx, Int32 option, IntPtr val, IntPtr size);

        [DllImport(LibraryName, EntryPoint = "oe_set_opt", CallingConvention = CCCdecl)]
        public static extern Int32 set_opt(IntPtr ctx, Int32 option, IntPtr val, UInt32 size);

        [DllImport(LibraryName, EntryPoint = "oe_read_reg", CallingConvention = CCCdecl)]
        public static extern Int32 read_reg(IntPtr ctx, UInt32 dev_idx, UInt32 addr, IntPtr val);

        [DllImport(LibraryName, EntryPoint = "oe_write_reg", CallingConvention = CCCdecl)]
        public static extern Int32 write_reg(IntPtr ctx, UInt32 dev_idx, UInt32 addr, UInt32 val);

        [DllImport(LibraryName, EntryPoint = "oe_read_frame", CallingConvention = CCCdecl, SetLastError = true)]
        public static extern Int32 read_frame(IntPtr ctx, out IntPtr frame);

        [DllImport(LibraryName, EntryPoint = "oe_destroy_frame", CallingConvention = CCCdecl)]
        public static extern Int32 destroy_frame(IntPtr frame);

        [DllImport(LibraryName, EntryPoint = "oe_error_str", CallingConvention = CCCdecl)]
        public static extern IntPtr error_str(Int32 err);

        //[DllImport(LibraryName, EntryPoint = "oe_device_str", CallingConvention = CCCdecl)]
        //public static extern IntPtr device_str(Int32 id);
    }
}
