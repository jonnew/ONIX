using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace oe
{
    using lib;
    
    public class Context : SafeHandleZeroOrMinusOneIsInvalid
    {
        protected Context() 
        : base(true)
        {
        }

        public Context(string config_path = NativeMethods.DefaultConfigPath,
                       string read_path = NativeMethods.DefaultReadPath,
                       string signal_path = NativeMethods.DefaultSignalPath)
        :base (true)
        {
            // Create context
            handle = NativeMethods.oe_create_ctx(); // this.handle is IntPtr wrapped by the SafeHandle
            if (handle == IntPtr.Zero)
            {
                throw new InvalidProgramException("oe_create_ctx");
            }

            // Set stream paths
            SetOption(Option.CONFIGSTREAMPATH, config_path);
            SetOption(Option.READSTREAMPATH, read_path);
            SetOption(Option.SIGNALSTREAMPATH, signal_path);

            int rc = NativeMethods.oe_init_ctx(handle);
            if (rc != 0) { throw new OEException(rc); }

            // Populate device map
            int num_devs = GetOption(Option.NUMDEVICES);
            DeviceMap = new Dictionary<int, device_t>();
            int size_dev = Marshal.SizeOf(new device_t());
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
                    DeviceMap.Add(i, (device_t)Marshal.PtrToStructure(mem, typeof(device_t)));
                    mem = new IntPtr((long)mem + size_dev);
                }
            }
        }

        public static readonly uint DefaultIndex = 0;
        public uint Index = DefaultIndex;
        public readonly Dictionary<int, device_t> DeviceMap;
        private Object context_lock = new Object();

        protected override bool ReleaseHandle()
        {
            return (NativeMethods.oe_destroy_ctx(handle) == 0);
        }

        public uint DeviceID(uint device_index)
        {
            if (device_index < DeviceMap.Count)
                return DeviceMap[(int)device_index].id;
            else
                throw new OEException((int)Error.DEVIDX);
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
                    int rc = NativeMethods.oe_get_opt(handle, (int)option, value, option_len.Ptr);
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

        public void Reset()
        {
            SetOption(Context.Option.RESET, 1);
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
                    int rc = NativeMethods.oe_set_opt(handle, (int)opt, mem, (uint)size);
                    if (rc != 0) { throw new OEException(rc); }
                }
            }
        }

        // String SetOption
        private void SetOption(Option opt, string value)
        {
            int ssize;
            using (var str = DispoIntPtr.AllocString(value, out ssize))
            {
                lock (context_lock)
                {
                    // NB: +1 is for trailing null character
                    int rc = NativeMethods.oe_set_opt(handle, (int)opt, str, (uint)ssize + 1);
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
                    int rc = NativeMethods.oe_read_reg(handle, dev_idx, reg_addr, value);
                    if (rc != 0) { throw new OEException(rc); }
                    return (uint)Marshal.ReadInt32(value.Ptr);
                }
            }
        }

        public void WriteRegister(uint dev_idx, uint reg_addr, uint value)
        {
            lock (context_lock)
            {
                int rc = NativeMethods.oe_write_reg(handle, dev_idx, reg_addr, value);
                if (rc != 0) { throw new OEException(rc); }
            }
        }

        public Frame ReadFrame()
        {
            Frame frame;
            int rc = NativeMethods.oe_read_frame(handle, out frame);
            frame.Map(DeviceMap);
            if (rc != 0) { throw new OEException(rc); }
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
            MAXREADFRAMESIZE,
            MAXWRITEFRAMESIZE,
            RUNNING,
            RESET,
            SYSCLKHZ,
        }

    }
}