using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace oe
{
    //using DeviceMap = List<Device>;
    using lib;

    public class Context : IDisposable
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
            var device_map = GetDeviceMap(num_devs);
        }

        ~Context()
        {
            oepcie.destroy_ctx(ctx);
        }

        private IntPtr ctx;
        private oepcie.device_t[] device_map;

        uint deviceID(uint device_index)
        {
            if (device_index < device_map.Length)
                return device_map[device_index].id;
            else
                throw new OEException((int)oepcie.Error.DEVIDX);
        }

        // NB: There must be a way to make these generic, but its confusing with all the pointer-wrapper crap

        // Low-level GetOption
        private void GetOption(Option option, IntPtr value, ref int size)
        {
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
        }

        // Int32 GetOption
        public Int32 GetOption(Option option)
        {
            int size = Marshal.SizeOf(typeof(Int32));
            using (var value = DispoIntPtr.Alloc(size))
            {
                GetOption(option, value.Ptr, ref size);
                return Marshal.ReadInt32(value.Ptr);
            }
        }

        // DeviceMap GetOption
        private oepcie.device_t[] GetDeviceMap(int num_dev)
        {
            device_map = new oepcie.device_t[num_dev];
            int size_dev = Marshal.SizeOf(device_map[0]);
            int size = size_dev * num_dev;
            using (var value = DispoIntPtr.Alloc(size))
            {
                GetOption(Option.DEVICEMAP, value.Ptr, ref size);
                var mem = value.Ptr;

                // TODO: This seems very inefficient. We allocate memroy in value and then copy 
                // each element into device_map.  Would be better to directly provide device map's 
                // memory as buffer.
                for (int i = 0; i < num_dev; i++) {
                   device_map[i] = (oepcie.device_t)Marshal.PtrToStructure(mem, typeof(oepcie.device_t));
                    mem = new IntPtr((long)mem + size_dev);
                }

                return device_map;
            }
          
        }

        // Int32 SetOption
        public void SetOption(Option opt, int value)
        {
            int size = Marshal.SizeOf(typeof(Int32));
            using (var mem = DispoIntPtr.Alloc(size))
            {
                Marshal.WriteInt32(mem.Ptr, value);
                int rc = oepcie.set_opt(ctx, (int)opt, mem, (uint)size);
                if (rc != 0) { throw new OEException(rc); }
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
            unsafe {
                var value = DispoIntPtr.Alloc(sizeof(oepcie.frame_t));
                var mem = value.Ptr;
                int rc = oepcie.read_frame(ctx, out mem);
                var frame = new Frame(device_map, mem);
                if (rc != 0) { throw new OEException(rc); }
                return frame;
            }
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

        void IDisposable.Dispose()
        {
            oepcie.destroy_ctx(ctx);
        }


        // Not entierly sure what this is for. I guess because ctx could get GCed??
        //private void EnsureNotDisposed()
        //{
        //    if (ctx == IntPtr.Zero)
        //    {
        //        throw new ObjectDisposedException(GetType().FullName);
        //    }
        //}
    }
}
