using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace oe
{
    using DeviceMap = List<Device>;
    using lib;

    public class Context
    {
        public Context(string config_path = oepcie.DefaultConfigPath,
                       string read_path = oepcie.DefaultReadPath,
                       string signal_path = oepcie.DefaultSignalPath)
        {

            // Create context
            ctx = oepcie.create_ctx();
            if (ctx == IntPtr.Zero)
            {
                throw new InvalidProgramException("oe_create_ctx");
            }

            // Set stream paths
            SetOption(Option.CONFIGSTREAMPATH, config_path);
            SetOption(Option.READSTREAMPATH, read_path);
            SetOption(Option.SIGNALSTREAMPATH, signal_path);
            
            int rc = oepcie.init_ctx(ctx);
            if (rc != 0) { throw new OEException(rc); }

            // Populate device map
            int num_devs = GetOption(Option.NUMDEVICES);
            device_map = new DeviceMap(num_devs);
        }

        ~Context()
        {
            oepcie.destroy_ctx(ctx);
        }

        private IntPtr ctx;
        public readonly DeviceMap device_map;

        // NB: There must be a way to make these generic, but its confusing with all the pointer-wrapper crap
        // Get int option

        // Low-level GetOption
        private bool GetOption(Option option, IntPtr value, ref int size)
        {
            EnsureNotDisposed();

            using (var option_len = DispoIntPtr.Alloc(IntPtr.Size))
            {
                if (IntPtr.Size == 4)
                    Marshal.WriteInt32(option_len.Ptr, size);
                else if (IntPtr.Size == 8)
                    Marshal.WriteInt64(option_len.Ptr, (long)size);
                else
                    throw new PlatformNotSupportedException();

                int rc = oepcie.get_opt(ctx, (int)option, value, option_len.Ptr);
                if (rc != 0) { throw new OEException(rc); }

                if (IntPtr.Size == 4)
                    size = Marshal.ReadInt32(option_len.Ptr);
                else if (IntPtr.Size == 8)
                    size = (int)Marshal.ReadInt64(option_len.Ptr);
                else
                    throw new PlatformNotSupportedException();
            }

            return true;
        }

        // Int32 GetOption
        public Int32 GetOption(Option option)
        {
            int size = Marshal.SizeOf(typeof(Int32));
            using (var value = DispoIntPtr.Alloc(size))
            {
                if (GetOption(option, value.Ptr, ref size))
                {
                    return Marshal.ReadInt32(value.Ptr);
                }
                return default(Int32);
            }
        }

        // String SetOption
        public void SetOption(Option opt, string value)
        {
            int ssize;
            using (var path_ptr = DispoIntPtr.AllocString(value, out ssize))
            {
                int rc = oepcie.set_opt(ctx, (int)opt, path_ptr, (uint)ssize);
                if (rc != 0) { throw new OEException(rc); }
            }
        }

        public uint ReadRegister(uint dev_idx, uint reg_addr)
        {
            int size = Marshal.SizeOf(typeof(Int32));
            using (var value = DispoIntPtr.Alloc(size))
            {
                int rc = oepcie.read_reg(ctx, dev_idx, reg_addr, value);
                if (rc != 0) { throw new OEException(rc); }
                return (uint)Marshal.ReadInt32(value.Ptr);
            }
        }

        public void WriteRegister(uint dev_idx, uint reg_addr, uint value)
        {
            int rc = oepcie.write_reg(ctx, dev_idx, reg_addr, value);
            if (rc != 0) { throw new OEException(rc); }
        }

        public Frame ReadFrame()
        {
            var frame = new Frame(device_map);
            int rc = oepcie.read_frame(ctx, out frame.frame_mem);
            if (rc != 0) frame.marshall();
            
            return frame;
        }

        // NB: These need to be redeclared unfortuately
        public enum Option : int
        {
            CONFIGSTREAMPATH = 0,
            READSTREAMPATH,
            SIGNALSTREAMPATH,
            DEVICEMAP,
            NUMDEVICES,
            READFRAMESIZE,
            WRITEFRAMESIZE,
            RUNNING,
            RESET,
            SYSCLKHZ
        }

        // Not entierly sure what this is for. I guess because ctx could get GCed??
        private void EnsureNotDisposed()
        {
            if (ctx == IntPtr.Zero)
            {
                throw new ObjectDisposedException(GetType().FullName);
            }
        }
    }
}
