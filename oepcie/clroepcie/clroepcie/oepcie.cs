namespace oe.lib
{
    using System;
    using System.Runtime.InteropServices;

    public static unsafe class oepcie
    {
        public static readonly Version LibraryVersion;

        private const CallingConvention CCCdecl = CallingConvention.Cdecl;

        private const string LibraryName = "liboepcie";

        public const string DefaultConfigPath = "\\\\.\\xillybus_cmd_32";
        public const string DefaultReadPath = "\\\\.\\xillybus_data_read_32";
        public const string DefaultSignalPath = "\\\\.\\xillybus_async_read_8";

        // Make managed version of oe_frame_t
        [StructLayout(LayoutKind.Sequential)]
        public struct frame_t
        {
            public ulong clock;       // Base clock counter
            public ushort num_dev;     // Number of devices in frame
            public byte corrupt;       // Is this frame corrupt?
            public UIntPtr dev_idxs;   // Array of device indices in frame
            public uint dev_idxs_sz; // Size in bytes of dev_idxs buffer
            public UIntPtr dev_offs;   // Device data offsets within data block
            public uint dev_offs_sz; // Size in bytes of dev_idxs buffer
            public IntPtr data;         // Multi-device raw data block
            public uint data_sz;     // Size in bytes of data buffer
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
        public static extern Int32 get_opt(IntPtr ctx, Int32 option, IntPtr val, out UInt32 size);

        [DllImport(LibraryName, EntryPoint = "oe_set_opt", CallingConvention = CCCdecl)]
        public static extern Int32 set_opt(IntPtr ctx, Int32 option, IntPtr val, UInt32 size);

        [DllImport(LibraryName, EntryPoint = "oe_read_reg", CallingConvention = CCCdecl)]
        public static extern Int32 read_reg(IntPtr ctx, UInt32 dev_idx, UInt32 addr, out UInt32 val);

        [DllImport(LibraryName, EntryPoint = "oe_write_reg", CallingConvention = CCCdecl)]
        public static extern Int32 write_reg(IntPtr ctx, UInt32 dev_idx, UInt32 addr, UInt32 val);

        [DllImport(LibraryName, EntryPoint = "oe_read_frame", CallingConvention = CCCdecl)]
        //public static extern Int32 read_frame(IntPtr ctx, FrameT **frame);
        public static extern Int32 read_frame(IntPtr ctx, out IntPtr frame);

        [DllImport(LibraryName, EntryPoint = "oe_destroy_frame", CallingConvention = CCCdecl)]
        //public static extern Int32 destroy_frame(out FrameT frame);
        public static extern Int32 destroy_frame(IntPtr frame);

        [DllImport(LibraryName, EntryPoint = "oe_error_str", CallingConvention = CCCdecl)]
        public static extern IntPtr error_str(Int32 err);

        [DllImport(LibraryName, EntryPoint = "oe_device_str", CallingConvention = CCCdecl)]
        public static extern IntPtr device_str(Int32 err);
    }
}
