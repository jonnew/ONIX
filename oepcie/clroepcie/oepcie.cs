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
        private static extern IntPtr oe_create_ctx();
        public delegate IntPtr oe_create_ctx_delegate();
        public static readonly oe_create_ctx_delegate create_ctx = oe_create_ctx;

        [DllImport(LibraryName, EntryPoint = "oe_init_ctx", CallingConvention = CCCdecl)]
        private static extern Int32 oe_init_ctx(IntPtr ctx);
        public delegate Int32 oe_init_ctx_delegate(IntPtr ctx);
        public static readonly oe_init_ctx_delegate init_ctx = oe_init_ctx;

        [DllImport(LibraryName, EntryPoint = "oe_get_opt", CallingConvention = CCCdecl)]
        private static extern Int32 oe_get_opt(IntPtr ctx, Int32 option, IntPtr val, UIntPtr size);
        public delegate Int32 oe_get_opt_delegate(IntPtr ctx, Int32 option, IntPtr val, UIntPtr size);
        public static readonly oe_get_opt_delegate get_opt = oe_get_opt;

        [DllImport(LibraryName, EntryPoint = "oe_set_opt", CallingConvention = CCCdecl)]
        private static extern Int32 oe_set_opt(IntPtr ctx, Int32 option, IntPtr val, UInt32 size);
        public delegate Int32 oe_set_opt_delegate(IntPtr ctx, Int32 option, IntPtr val, UInt32 size);
        public static readonly oe_set_opt_delegate set_opt = oe_set_opt;

        [DllImport(LibraryName, EntryPoint = "oe_read_reg", CallingConvention = CCCdecl)]
        private static extern Int32 oe_read_reg(IntPtr ctx, UInt32 dev_idx, UInt32 addr, UIntPtr val);
        public delegate Int32 oe_read_reg_delegate(IntPtr ctx, UInt32 dev_idx, UInt32 addr, UIntPtr val);
        public static readonly oe_read_reg_delegate read_reg = oe_read_reg;

        [DllImport(LibraryName, EntryPoint = "oe_write_reg", CallingConvention = CCCdecl)]
        private static extern Int32 oe_write_reg(IntPtr ctx, UInt32 dev_idx, UInt32 addr, UInt32 val);
        public delegate Int32 oe_write_reg_delegate(IntPtr ctx, UInt32 dev_idx, UInt32 addr, UInt32 val);
        public static readonly oe_write_reg_delegate write_reg = oe_write_reg;

        [DllImport(LibraryName, EntryPoint = "oe_read", CallingConvention = CCCdecl)]
        private static extern Int32 oe_read(IntPtr ctx, IntPtr data, UInt32 size);
        public delegate Int32 oe_read_delegate(IntPtr ctx, IntPtr data, UInt32 size);
        public static readonly oe_read_delegate read = oe_read;

        [DllImport(LibraryName, EntryPoint = "oe_error_str", CallingConvention = CCCdecl)]
        private static extern IntPtr oe_error_str(Int32 err);
        public delegate IntPtr oe_error_str_delegate(Int32 err);
        public static readonly oe_error_str_delegate error_str = oe_error_str;

        [DllImport(LibraryName, EntryPoint = "oe_device_str", CallingConvention = CCCdecl)]
        private static extern IntPtr oe_device_str(Int32 err);
        public delegate IntPtr oe_device_str_delegate(Int32 err);
        public static readonly oe_device_str_delegate device_str = oe_device_str;
    }
}
