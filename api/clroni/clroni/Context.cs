namespace oni
{
    using System;
    using System.Text;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using Microsoft.Win32.SafeHandles;

    using lib;
    using System.Threading.Tasks;

    public class Context : SafeHandleZeroOrMinusOneIsInvalid
    {
        protected Context() 
        : base(true)
        {
        }

        public Context(string driver_name, int host_idx, params object[] args)
        :base (true)  // this.handle is IntPtr wrapped by the SafeHandle
        {
            // Create context
            handle = NativeMethods.oni_create_ctx(driver_name, host_idx);
            if (handle == IntPtr.Zero)
            {
                throw new InvalidProgramException("oni_create_ctx");
            }
            
            // Set driver options
            if (args.Length % 2 != 0) 
            {
                throw new ArgumentException("args must be a list of alternating driver options and values.");
            }
            
            for (int i = 0; i < args.Length; i+=2)
                SetStringOption((int)args[i], (string)args[i+1], true);

            // Attempt to initialize context, but timeout if it takes longer than 2 seconds
            var task = Task.Run(() =>
            {
                return NativeMethods.oni_init_ctx(handle);
            });

            bool isCompletedSuccessfully = task.Wait(TimeSpan.FromSeconds(2));

            if (isCompletedSuccessfully)
            {
                if (task.Result != 0) { throw new ONIException(task.Result); }
            }
            else
            {
                throw new TimeoutException("Context intialization timed out.");
            }

            // Get context metadata
            SystemClockHz = GetIntOption((int)Option.SYSCLKHZ);
            MaxReadFrameSize = GetIntOption((int)Option.MAXREADFRAMESIZE);

            // Populate device map
            int num_devs = GetIntOption((int)Option.NUMDEVICES);
            DeviceMap = new Dictionary<int, device_t>();
            int size_dev = Marshal.SizeOf(new device_t());
            int size = size_dev * num_devs; // bytes required to read map

            var map = GetOption((int)Option.DEVICEMAP, size);

            // TODO: This seems very inefficient. We allocate memory in value
            // and then copy each element into device_map.  Would be better to
            // directly provide device map's memory as buffer.
            for (int i = 0; i < num_devs; i++)
            {
                DeviceMap.Add(i, (device_t)Marshal.PtrToStructure(map, typeof(device_t)));
                map = new IntPtr((long)map + size_dev);
            }

            destroyed = false;
        }

        public static readonly uint DefaultIndex = 0;
        public uint Index = DefaultIndex;
        public readonly Dictionary<int, device_t> DeviceMap;
        readonly object context_lock = new object();

        public readonly int SystemClockHz = 0;
        public readonly int MaxReadFrameSize = 0;
        public readonly int ReadBytes = 0;
        bool destroyed = true;
        readonly object openContextsLock = new object();

        protected override bool ReleaseHandle()
        {
            lock (context_lock)
            {
                if (destroyed)
                {
                    return true;
                }
                else
                {
                    var rc = NativeMethods.oni_destroy_ctx(handle) == 0;
                    destroyed = true;
                    return rc;
                }
            }
        }

        public void Destroy()
        {
            if (!ReleaseHandle())
                throw new ONIException((int)oni.lib.Error.CLOSEFAIL);
        }

        public int DeviceID(uint device_index)
        {
            if (device_index < DeviceMap.Count)
                return DeviceMap[(int)device_index].id;
            else
                throw new ONIException((int)Error.DEVIDX);
        }

        // GetOption
        private IntPtr GetOption(int option, int size, bool drv_opt = false)
        {
            // NB: If I dont do all this wacky stuff, the size 
            // parameter ends up being too wrong for 64-bit compilation.
            var sz = Marshal.AllocHGlobal(IntPtr.Size);
            if (IntPtr.Size == 4) Marshal.WriteInt32(sz, size); else Marshal.WriteInt64(sz, size);
            var value = Marshal.AllocHGlobal(size);

            int rc = 0;
            if (!drv_opt)
                rc = NativeMethods.oni_get_opt(handle, (int)option, value, sz);
            else
                rc = NativeMethods.oni_get_driver_opt(handle, (int)option, value, sz);

            if (rc != 0) { throw new ONIException(rc); }
            return value;
        }

        // Int32 GetOption
        private int GetIntOption(int option, bool drv_opt = false)
        {
            var value = GetOption(option, Marshal.SizeOf(typeof(int)), drv_opt);
            return Marshal.ReadInt32(value); 
        }

        // String GetOption
        private string GetStringOption(int option, bool drv_opt = false)
        {
            var sz = Marshal.AllocHGlobal(IntPtr.Size);
            if (IntPtr.Size == 4) Marshal.WriteInt32(sz, 1000); else Marshal.WriteInt64(sz, 1000);
            var str = new StringBuilder(1000);
            int rc = 0;
            if (!drv_opt)
                rc = NativeMethods.oni_get_opt(handle, (int)option, str, sz);
            else
                rc = NativeMethods.oni_get_driver_opt(handle, (int)option, str, sz);
            if (rc != 0) { throw new ONIException(rc); }
            return str.ToString();
        }

        // Int32 SetOption
        private void SetIntOption(int opt, int value, bool drv_opt = false)
        {
            var val = Marshal.AllocHGlobal(IntPtr.Size);
            if (IntPtr.Size == 4) Marshal.WriteInt32(val, value); else Marshal.WriteInt64(val, value);
            int rc = 0;
            if (!drv_opt)
                rc = NativeMethods.oni_set_opt(handle, opt, val, 4);
            else
                rc = NativeMethods.oni_set_driver_opt(handle, opt, val, 4);

            if (rc != 0) { throw new ONIException(rc); }
        }

        // String SetOption
        private void SetStringOption(int opt, string value, bool drv_opt)
        {
            int rc = 0;
            if (!drv_opt)
                rc = NativeMethods.oni_set_opt(handle, (int)opt, value, value.Length + 1);
            else
                rc = NativeMethods.oni_set_driver_opt(handle, (int)opt, value, value.Length + 1);
            if (rc != 0) { throw new ONIException(rc); }
        }

        public void Start()
        {
            SetIntOption((int)Context.Option.RUNNING, 1);
        }

        public void Stop()
        {
            SetIntOption((int)Context.Option.RUNNING, 0);
        }

        public void Reset()
        {
            SetIntOption((int)Context.Option.RESET, 1);
        }

        public void SetBlockReadSize(int block_read_size)
        {
            SetIntOption((int)Context.Option.BLOCKREADSIZE, block_read_size);
        }

        public uint ReadRegister(uint dev_idx, uint reg_addr)
        {
            lock (context_lock)
            {
                var val = Marshal.AllocHGlobal(4);
                int rc = NativeMethods.oni_read_reg(handle, dev_idx, reg_addr, val);
                if (rc != 0) { throw new ONIException(rc); }
                return (uint)Marshal.ReadInt32(val);
            }
        }

        public void WriteRegister(uint dev_idx, uint reg_addr, uint value)
        {
            lock (context_lock)
            {
                int rc = NativeMethods.oni_write_reg(handle, dev_idx, reg_addr, value);
                if (rc != 0) { throw new ONIException(rc); }
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
        //            int rc = NativeMethods.oni_write_reg(handle, dev_idx, reg_addr, value);
        //            if (rc != 0) { throw new ONIException(rc); }
        //        }
        //    }, ct);

        //    if (await Task.WhenAny(t, Task.Delay(RegisterTimeoutMillis)) == t)
        //    {
        //        return;   
        //    }
        //    else
        //    {
        //        throw new ONIException((int)Error.WRITEFAILURE);
        //    }
        //}

        public Frame ReadFrame()
        {
            Frame frame;
            int rc = NativeMethods.oni_read_frame(handle, out frame);
            frame.Map(DeviceMap);
            if (rc < 0) { throw new ONIException(rc); }
            return frame;
        }

        // Byte array write specialization
        public void Write(uint dev_idx, IntPtr data, int length)
        {
            int rc = NativeMethods.oni_write(handle, dev_idx, data, (uint)length);
            if (rc < 0) { throw new ONIException(rc); }
        }

        // TODO: C# 7.3
        // Generic write with array type cast
        //public void Write<T>(uint dev_idx, T[] data) where T : unmanaged
        //{
        //    var byte_data = Array.ConvertAll(data, item => (byte)item);
        //    int rc = NativeMethods.oni_write(handle, dev_idx, byte_data, byte_data.Length);
        //    if (rc < 0) { throw new ONIException(rc); }
        //}

        // NB: These need to be redeclared unfortuately
        public enum Option : int
        {
            DEVICEMAP = 0,
            NUMDEVICES,
            MAXREADFRAMESIZE,
            RUNONRESET,
            RUNNING,
            RESET,
            SYSCLKHZ,
            BLOCKREADSIZE
        }
    }
}
