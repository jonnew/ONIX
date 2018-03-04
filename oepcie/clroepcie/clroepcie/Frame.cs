using System;
using System.Collections.Generic;

namespace oe
{
    using DeviceMap = List<Device>;
    using lib;
    using System.Runtime.InteropServices;

    public unsafe class Frame
    {
        public Frame(DeviceMap dev_map)
        {
            this.dev_map = dev_map;
        }

        ~Frame()
        {
            oepcie.destroy_frame(frame_mem);
        }

        internal void marshall()
        {
            frame = (oepcie.frame_t)Marshal.PtrToStructure(frame_mem, typeof(oepcie.frame_t));
        }

        public UInt64 time() { return frame.clock; }
        public bool corrupt() { return frame.corrupt != 0; }

        List<T> data<T>(int dev_idx)
        {
            // Smash data into list somehow

            return new List<T>();
        }

        // NB: I think that since DeviceMap is an object, we automatically only store a reference here.
        public readonly DeviceMap dev_map;
        internal IntPtr frame_mem = IntPtr.Zero;
        internal oepcie.frame_t frame;
        //internal oepcie.frame_t *frame; // NB: I think this is not visiable outside dll??
    }
}
