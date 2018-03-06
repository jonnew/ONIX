using System;
using System.Collections.Generic;

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
            device_map.Capacity = num_devs;
        }

        ~Context()
        {
            oepcie.destroy_ctx(ctx);
        }

        private IntPtr ctx;
        public readonly DeviceMap device_map;

        // NB: There must be a way to make these generic, but its confusing with all the pointer-wrapper crap
        // Get int option
        public int GetOption(Option opt) // Int version
        {
            UInt32 sz;
            IntPtr value = (IntPtr)0;
            int rc = oepcie.get_opt(ctx, (int)opt, value, out sz);
            if (rc != 0) { throw new OEException(rc); }
            return (int)value;
        }

        public void SetOption(Option opt, string value) // String version
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
            uint value;
            int rc = oepcie.read_reg(ctx, dev_idx, reg_addr, out value);
            if (rc != 0) { throw new OEException(rc); }
            return value;
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
    }
}
