namespace oe.lib
{
    using System;
    using System.Runtime.InteropServices;

    public static unsafe class oepcie
    {
        public static readonly Version LibraryVersion;

        private const CallingConvention CCCdecl = CallingConvention.Cdecl;

        private const string LibraryName = "liboepcie";

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
        private static extern void oe_version(out int major, out int minor, out int patch);

        // (2) Describe the extern function using a delegate
        private delegate void oe_version_delegate(out int major, out int minor, out int patch);

        // (3) Save and return the managed delegate to the unmanaged function
        //     This static readonly field definition allows to be
        //     initialized and possibly redirected by the static constructor.
        //     (usually public, but we can access this through LibraryVersion)
        private static readonly oe_version_delegate version = oe_version;

        // Repeat for the rest of the API
        [DllImport(LibraryName, EntryPoint = "oe_create_ctx", CallingConvention = CCCdecl)]
        public static extern IntPtr create_ctx();

        [DllImport(LibraryName, EntryPoint = "oe_init_ctx", CallingConvention = CCCdecl)]
        public static extern Int32 init_ctx(IntPtr ctx);

        [DllImport(LibraryName, EntryPoint = "oe_destry_ctx", CallingConvention = CCCdecl)]
        public static extern Int32 destroy_ctx(IntPtr ctx);

        [DllImport(LibraryName, EntryPoint = "oe_get_opt", CallingConvention = CCCdecl)]
        public static extern Int32 get_opt(IntPtr ctx, Int32 option, IntPtr val, UIntPtr size);

        [DllImport(LibraryName, EntryPoint = "oe_set_opt", CallingConvention = CCCdecl)]
        public static extern Int32 set_opt(IntPtr ctx, Int32 option, IntPtr val, UInt32 size);

        [DllImport(LibraryName, EntryPoint = "oe_read_reg", CallingConvention = CCCdecl)]
        public static extern Int32 read_reg(IntPtr ctx, UInt32 dev_idx, UInt32 addr, UIntPtr val);

        [DllImport(LibraryName, EntryPoint = "oe_write_reg", CallingConvention = CCCdecl)]
        public static extern Int32 write_reg(IntPtr ctx, UInt32 dev_idx, UInt32 addr, UInt32 val);

        [DllImport(LibraryName, EntryPoint = "oe_read_frame", CallingConvention = CCCdecl)]
        public static extern Int32 read(IntPtr ctx, IntPtr frame);

        [DllImport(LibraryName, EntryPoint = "oe_error_str", CallingConvention = CCCdecl)]
        public static extern IntPtr error_str(Int32 err);

        [DllImport(LibraryName, EntryPoint = "oe_device_str", CallingConvention = CCCdecl)]
        public static extern IntPtr device_str(Int32 err);
    }
}
