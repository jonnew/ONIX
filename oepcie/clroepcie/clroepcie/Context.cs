using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using MapDevice = System.Tuple<int, oe.lib.oepcie.device_t>;

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
            DeviceMap = new List<MapDevice>();
            int size_dev = Marshal.SizeOf(new oe.lib.oepcie.device_t());
            int size = size_dev * num_devs;
            using (var value = DispoIntPtr.Alloc(size))
            {
                GetOption(Option.DEVICEMAP, value.Ptr, ref size);
                var mem = value.Ptr;

                // TODO: This seems very inefficient. We allocate memroy in value and then copy 
                // each element into device_map.  Would be better to directly provide device map's 
                // memory as buffer.
                for (int i = 0; i < num_devs; i++)
                {
                    DeviceMap.Add(new MapDevice(i, (oepcie.device_t)Marshal.PtrToStructure(mem, typeof(oepcie.device_t))));
                    mem = new IntPtr((long)mem + size_dev);
                }

                // We are not disposed
                disposed = false;
            }
        }

        ~Context()
        {
            Dispose(false);  // TODO: This does not seem to do anything. Why is it called with false arg?
        }

        public static readonly uint DefaultIndex = 0;
        public uint Index = DefaultIndex;
        private IntPtr ctx;
        public readonly List<MapDevice> DeviceMap;
        private bool disposed;
        private Object context_lock = new Object();

        public uint deviceID(uint device_index)
        {
            if (device_index < DeviceMap.Count)
                return DeviceMap[(int)device_index].Item2.id;
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

                lock (context_lock)
                {
                    int rc = oepcie.get_opt(ctx, (int)option, value, option_len.Ptr);
                    if (rc != 0) { throw new OEException(rc); }
                }

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

        public void Start()
        {
            SetOption(Context.Option.RUNNING, 1);
        }

        public void Stop()
        {
            SetOption(Context.Option.RUNNING, 0);
        }

        // Int32 SetOption
        private void SetOption(Option opt, int value)
        {
            int size = Marshal.SizeOf(typeof(Int32));
            using (var mem = DispoIntPtr.Alloc(size))
            {
                Marshal.WriteInt32(mem.Ptr, value);
                lock (context_lock)
                {
                    int rc = oepcie.set_opt(ctx, (int)opt, mem, (uint)size);
                    if (rc != 0) { throw new OEException(rc); }
                }
            }  
        }

        // String SetOption
        private void SetOption(Option opt, string value)
        {
            int ssize;
            using (var path_ptr = DispoIntPtr.AllocString(value, out ssize))
            {
                lock (context_lock)
                {
                    int rc = oepcie.set_opt(ctx, (int)opt, path_ptr, (uint)ssize);
                    if (rc != 0) { throw new OEException(rc); }
                }

            }
        }

        public uint ReadRegister(uint dev_idx, uint reg_addr)
        {
            int size = Marshal.SizeOf(typeof(Int32));
            using (var value = DispoIntPtr.Alloc(size))
            {
                lock (context_lock)
                {
                    int rc = oepcie.read_reg(ctx, dev_idx, reg_addr, value);
                    if (rc != 0) { throw new OEException(rc); }
                    return (uint)Marshal.ReadInt32(value.Ptr);
                }
            }
        }

        public void WriteRegister(uint dev_idx, uint reg_addr, uint value)
        {
            lock (context_lock)
            {
                int rc = oepcie.write_reg(ctx, dev_idx, reg_addr, value);
                if (rc != 0) { throw new OEException(rc); }
            }
        }

        public Frame ReadFrame()
        {
            unsafe {
                var value = DispoIntPtr.Alloc(sizeof(oepcie.frame_t));
                var mem = value.Ptr;
                lock (context_lock)
                {
                    int rc = oepcie.read_frame(ctx, out mem);
                    if (rc != 0) { throw new OEException(rc); }
                }
                var frame = new Frame(DeviceMap, mem);
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

        public void Close()
        {
            Dispose(true);
            GC.SuppressFinalize(this); // I guess this is because we are calling Dispose manually here

        }

        void Dispose(bool disposing)
        {
            if (!disposed)
            {
                int rc = oepcie.destroy_ctx(ctx); // Free resources held by ctx. This does not seem to happen correctly all the time.
                if (rc != 0) { throw new OEException(rc); }
                disposed = true;
            }
        }

        void IDisposable.Dispose()
        {
            Close(); 
        }

    }
}
