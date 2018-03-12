using System;
using System.Collections.Generic;

namespace oe
{
    using lib;
    using System.Runtime.InteropServices;

    public unsafe class Frame : IDisposable
    {
        public Frame(oepcie.device_t[] dev_map, IntPtr frame_mem)
        {
            this.dev_map = dev_map;
            this.frame_mem = frame_mem;
            frame = (oepcie.frame_t)Marshal.PtrToStructure(frame_mem, typeof(oepcie.frame_t));
        }

        ~Frame()
        {
            oepcie.destroy_frame(frame_mem);
        }

        public UInt64 time() { return frame.clock; }
        public bool corrupt() { return frame.corrupt != 0; }

        public int contains(int dev_idx)
        {
            for (int i = 0; i < frame.num_dev; i++) {
            if (*(frame.dev_idxs + i) == dev_idx)
                return i;
            }
            return -1;
        }

        // NB: This seems horribly inefficient, but I really have no idea
        public T[] data<T>(int dev_idx) where T : IConvertible
        {
            // Smash data into list somehow
            // Get the byte size of the array
            int frame_idx = contains(dev_idx);
            if (frame_idx == -1)
            {
                throw new OEException(-9); // OE_EDEVIDX
            }

            var num_bytes = dev_map[dev_idx].read_size;
            var byte_offset = *(frame.dev_offs + frame_idx);
            var t_size = Marshal.SizeOf(typeof(T));
            var n = num_bytes / t_size;

            var output = new T[n];
            int j = 0;
            var start_ptr = frame.data + byte_offset;
            for (int i = 0; i < n; i ++)
            {
                output[i] = (T)Convert.ChangeType(*(start_ptr + j), typeof(T));
                j += t_size;
            }
            return output;
        }

        void IDisposable.Dispose()
        {
            oepcie.destroy_frame(frame_mem);
        }

        // NB: I think that since DeviceMap is an object, we automatically only store a reference here.
        public readonly oepcie.device_t[] dev_map;
        private oepcie.frame_t frame;
        private IntPtr frame_mem;
    }
}