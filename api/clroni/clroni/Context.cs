namespace oni
{
    using System;
    using System.Text;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using Microsoft.Win32.SafeHandles;

    using lib;

    public unsafe class Context : SafeHandleZeroOrMinusOneIsInvalid
    {
        // NB: These need to be redeclared unfortunately
        public enum Option : int
        {
            DEVICETABLE = 0,
            NUMDEVICES,
            RUNNING,
            RESET,
            SYSCLKHZ,
            ACQCLKHZ,
            RESETACQCOUNTER,
            HWADDRESS,
            MAXREADFRAMESIZE,
            MAXWRITEFRAMESIZE,
            BLOCKREADSIZE,
            BLOCKWRITESIZE
        }

        // Hardware constants
        public readonly string HostDriver;
        public readonly int HostIndex;
        public readonly Dictionary<uint, device_t> DeviceTable;
        public readonly uint SystemClockHz = 0;
        public readonly uint AcquisitionClockHz = 0;
        public readonly uint MaxReadFrameSize = 0;
        public readonly uint MaxWriteFrameSize = 0;

        readonly object context_lock = new object();

        public Context(string driver_name, int host_index)
        : base (true)  // this.handle is IntPtr wrapped by the SafeHandle
        {
            // Create context
            handle = NativeMethods.oni_create_ctx(driver_name);
            if (handle == IntPtr.Zero)
            {
                throw new InvalidProgramException("oni_create_ctx");
            }

            var rc = NativeMethods.oni_init_ctx(handle, host_index);
            if (rc != 0) { throw new ONIException(rc); }

            // Get context metadata
            HostDriver = driver_name;
            HostIndex = host_index;
            SystemClockHz = (uint)GetIntOption((int)Option.SYSCLKHZ);
            AcquisitionClockHz = (uint)GetIntOption((int)Option.ACQCLKHZ);
            MaxReadFrameSize = (uint)GetIntOption((int)Option.MAXREADFRAMESIZE);
            MaxWriteFrameSize = (uint)GetIntOption((int)Option.MAXWRITEFRAMESIZE);

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
        }

        protected override bool ReleaseHandle()
        {
            return NativeMethods.oni_destroy_ctx(handle) == 0;
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
                rc = NativeMethods.oni_get_opt(handle, option, value, sz);
            else
                rc = NativeMethods.oni_get_driver_opt(handle, option, value, sz);

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

        public void Start(bool reset_frame_clock = true)
        {
            if (reset_frame_clock)
                SetIntOption((int)Option.RESETACQCOUNTER, 2);
            else
                SetIntOption((int)Option.RUNNING, 1);
        }

        public void Stop()
        {
            SetIntOption((int)Option.RUNNING, 0);
        }

        public void Reset()
        {
            SetIntOption((int)Option.RESET, 1);
        }

        public void ResetFrameClock()
        {
            SetIntOption((int)Option.RESETACQCOUNTER, 1);
        }

        //public void SetBlockReadSize(int block_size)
        //{
        //    SetIntOption((int)Option.BLOCKREADSIZE, block_size);
        //}

        //public void SetBlockWriteSize(int block_size)
        //{
        //    SetIntOption((int)Option.BLOCKWRITESIZE, block_size);
        //}

        public bool Running
        {
            get
            {
                return GetIntOption((int)Option.RUNNING) > 0;
            }
        }

        public int HardwareAddress
        {
            get
            {
                return GetIntOption((int)Option.HWADDRESS);
            }
            set
            {
                SetIntOption((int)Option.HWADDRESS, value);
            }
        }

        public int BlockReadSize
        {
            get
            {
                return GetIntOption((int)Option.BLOCKREADSIZE);
            }
            set
            {
                SetIntOption((int)Option.BLOCKREADSIZE, value);
            }
        }

        public int BlockWriteSize
        {
            get
            {
                return GetIntOption((int)Option.BLOCKWRITESIZE);
            }
            set
            {
                SetIntOption((int)Option.BLOCKWRITESIZE, value);
            }
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
    }
}
