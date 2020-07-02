namespace oni
{
    using System;
    using System.Text;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using Microsoft.Win32.SafeHandles;

    using lib;
    using System.Linq;

    public unsafe class Context : SafeHandleZeroOrMinusOneIsInvalid
    {
        protected Context()
        : base(true) { }

        public Context(string driver_name, int host_idx)
        :base (true)  // this.handle is IntPtr wrapped by the SafeHandle
        {
            // Create context
            handle = NativeMethods.oni_create_ctx(driver_name);
            if (handle == IntPtr.Zero)
            {
                throw new InvalidProgramException("oni_create_ctx");
            }

            var rc = NativeMethods.oni_init_ctx(handle, host_idx);
            if (rc != 0) { throw new ONIException(rc); }

            // Get context metadata
            SystemClockHz = (uint)GetIntOption((int)Option.SYSCLKHZ);
            MaxReadFrameSize = (uint)GetIntOption((int)Option.MAXREADFRAMESIZE);

            // Populate device table
            int num_devs = GetIntOption((int)Option.NUMDEVICES);
            DeviceTable = new Dictionary<uint, device_t>(num_devs);
            int size_dev = Marshal.SizeOf(new device_t());
            int size = size_dev * num_devs; // bytes required to read table

            var table = GetOption((int)Option.DEVICETABLE, size);

            // TODO: This seems very inefficient. We allocate memory in value
            // and then copy each element into device table.  Would be better to
            // directly provide device table's memory as buffer.
            for (int i = 0; i < num_devs; i++)
            {
                var d = (device_t)Marshal.PtrToStructure(table, typeof(device_t));
                DeviceTable.Add(d.idx, d);
                table = new IntPtr((long)table + size_dev);
            }

            destroyed = false;
        }

        public static readonly uint DefaultIndex = 0;
        public uint Index = DefaultIndex;
        public readonly Dictionary<uint, device_t> DeviceTable;
        readonly object context_lock = new object();

        public readonly uint SystemClockHz = 0;
        public readonly uint MaxReadFrameSize = 0;
        public readonly uint ReadBytes = 0;
        bool destroyed = true;

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

        // GetOption
        private IntPtr GetOption(int option, int size, bool drv_opt = false)
        {
            // NB: If I don't do all this wacky stuff, the size
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
            if (rc < 0) { throw new ONIException(rc); }
            return frame;
        }

        public void Write<T>(uint dev_idx, T data) where T : struct
        {
            Write(dev_idx, new T[] { data });
        }

        public void Write<T> (uint dev_idx, T[] data) where T : struct
        {
            Frame frame;
            int rc = NativeMethods.oni_create_frame(handle, out frame, dev_idx, (uint)Buffer.ByteLength(data));
            if (rc < 0) { throw new ONIException(rc); }
            frame.SetData(data);

            rc = NativeMethods.oni_write_frame(handle, frame);
            if (rc < 0) { throw new ONIException(rc); }
        }

        public void Write(uint dev_idx, IntPtr data, int data_size)
        {
            Frame frame;
            int rc = NativeMethods.oni_create_frame(handle, out frame, dev_idx, (uint)data_size);
            if (rc < 0) { throw new ONIException(rc); }
            frame.SetData(data, data_size);

            rc = NativeMethods.oni_write_frame(handle, frame);
            if (rc < 0) { throw new ONIException(rc); }
        }

        // NB: These need to be redeclared unfortunately
        public enum Option : int
        {
            DEVICETABLE = 0,
            NUMDEVICES,
            MAXREADFRAMESIZE,
            RUNONRESET,
            RUNNING,
            RESET,
            SYSCLKHZ,
            BLOCKREADSIZE,
            BLOCKWRITESIZE
        }
    }
}
