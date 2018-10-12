namespace oe
{
    using System;
    using System.Text;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using Microsoft.Win32.SafeHandles;

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
            SetStringOption(Option.CONFIGSTREAMPATH, config_path);
            SetStringOption(Option.READSTREAMPATH, read_path);
            SetStringOption(Option.SIGNALSTREAMPATH, signal_path);

            int rc = NativeMethods.oe_init_ctx(handle);
            if (rc != 0) { throw new OEException(rc); }

            // Get context metadata
            SystemClockHz = GetIntOption(Option.SYSCLKHZ);
            AcquisitionClockHz = GetIntOption(Option.ACQCLKHZ);
            MaxReadFrameSize = GetIntOption(Option.MAXREADFRAMESIZE); // TODO: This is still not correct for some reason
            MaxWriteFrameSize = GetIntOption(Option.MAXWRITEFRAMESIZE);

            // Populate device map
            int num_devs = GetIntOption(Option.NUMDEVICES);
            DeviceMap = new Dictionary<int, device_t>();
            int size_dev = Marshal.SizeOf(new device_t());
            int size = size_dev * num_devs; // bytes required to read map

            var map = GetOption(Option.DEVICEMAP, size);

            // TODO: This seems very inefficient. We allocate memory in value and then copy
            // each element into device_map.  Would be better to directly provide device map's
            // memory as buffer.
            for (int i = 0; i < num_devs; i++)
            {
                DeviceMap.Add(i, (device_t)Marshal.PtrToStructure(map, typeof(device_t)));
                map = new IntPtr((long)map + size_dev);
            }
        }

        public static readonly uint DefaultIndex = 0;
        public uint Index = DefaultIndex;
        public readonly Dictionary<int, device_t> DeviceMap;
        private object context_lock = new object();

        public readonly int SystemClockHz = 0;
        public readonly int AcquisitionClockHz = 0;
        public readonly int MaxReadFrameSize = 0;
        public readonly int MaxWriteFrameSize = 0;

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

        // GetOption
        private IntPtr GetOption(Option option, int size)
        {
            // NB: If I dont do all this wacky stuff, the size 
            // parameter ends up being too wrong for 64-bit compilation.
            var sz = Marshal.AllocHGlobal(IntPtr.Size);
            if (IntPtr.Size == 4) Marshal.WriteInt32(sz, size); else Marshal.WriteInt64(sz, size);
            var value = Marshal.AllocHGlobal(size);
            int rc = NativeMethods.oe_get_opt(handle, (int)option, value, sz);
            if (rc != 0) { throw new OEException(rc); }
            return value;
        }

        // Int32 GetOption
        private int GetIntOption(Option option)
        {
            var value = GetOption(option, Marshal.SizeOf(typeof(int)));
            return Marshal.ReadInt32(value); 
        }

        // String GetOption
        private string GetStringOption(Option option)
        {
            var sz = Marshal.AllocHGlobal(IntPtr.Size);
            if (IntPtr.Size == 4) Marshal.WriteInt32(sz, 1000); else Marshal.WriteInt64(sz, 1000);
            var str = new StringBuilder(1000);
            int rc = NativeMethods.oe_get_opt(handle, (int)option, str, sz);
            if (rc != 0) { throw new OEException(rc); }
            return str.ToString();
        }

        // Int32 SetOption
        private void SetIntOption(Option opt, int value)
        {
            var val = Marshal.AllocHGlobal(IntPtr.Size);
            if (IntPtr.Size == 4) Marshal.WriteInt32(val, value); else Marshal.WriteInt64(val, value);
            int rc = NativeMethods.oe_set_opt(handle, (int)opt, val, 4);
            if (rc != 0) { throw new OEException(rc); }
        }

        // String SetOption
        private void SetStringOption(Option opt, string value)
        {
            int rc = NativeMethods.oe_set_opt(handle, (int)opt, value, value.Length + 1);
            if (rc != 0) { throw new OEException(rc); }
        }

        public void Start()
        {
            SetIntOption(Context.Option.RUNNING, 1);
        }

        public void Stop()
        {
            SetIntOption(Context.Option.RUNNING, 0);
        }

        public void Reset()
        {
            SetIntOption(Context.Option.RESET, 1);
        }

        public uint ReadRegister(uint dev_idx, uint reg_addr)
        {
            lock (context_lock)
            {
                var val = Marshal.AllocHGlobal(4);
                int rc = NativeMethods.oe_read_reg(handle, dev_idx, reg_addr, val);
                if (rc != 0) { throw new OEException(rc); }
                return (uint)Marshal.ReadInt32(val);
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

        // TODO: timeout read/write register?
        // private static readonly int RegisterTimeoutMillis = 5000;
        //public async void WriteRegister(uint dev_idx, uint reg_addr, uint value)
        //{
        //    var ts = new CancellationTokenSource();
        //    CancellationToken ct = ts.Token;
        //    var t = Task.Factory.StartNew(() =>
        //    {
        //        lock (context_lock)
        //        {
        //            int rc = NativeMethods.oe_write_reg(handle, dev_idx, reg_addr, value);
        //            if (rc != 0) { throw new OEException(rc); }
        //        }
        //    }, ct);

        //    if (await Task.WhenAny(t, Task.Delay(RegisterTimeoutMillis)) == t)
        //    {
        //        return;
        //    }
        //    else
        //    {
        //        throw new OEException((int)Error.WRITEFAILURE);
        //    }
        //}

        public Frame ReadFrame()
        {
            Frame frame;
            int rc = NativeMethods.oe_read_frame(handle, out frame);
            frame.Map(DeviceMap);
            if (rc < 0) { throw new OEException(rc); }
            return frame;
        }

        // NB: These need to be redeclared unfortuately
        public enum Option : int
        {
            CONFIGSTREAMPATH = 0,
            READSTREAMPATH,
            WRITESREAMPATH,
            SIGNALSTREAMPATH,
            DEVICEMAP,
            NUMDEVICES,
            MAXREADFRAMESIZE,
            MAXWRITEFRAMESIZE,
            RUNNING,
            RESET,
            SYSCLKHZ,
            ACQCLKHZ
        }

    }
}