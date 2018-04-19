using System;
using System.Collections.Generic;

namespace oe
{
    using lib;
    using System.Runtime.InteropServices;

    public unsafe class Frame : IDisposable
    {
        public Frame(Dictionary<int, oepcie.device_t> dev_map, IntPtr frame_mem)
        {
            // Deep copy of frame memory into managed memory
            this.frame_mem = frame_mem;
            frame = (oepcie.frame_t)Marshal.PtrToStructure(frame_mem, typeof(oepcie.frame_t));

            // Get device map reference
            DeviceMap = dev_map;

            // Get devices in this frame
            DeviceIndices = new List<int>(frame.num_dev);
            for (int i = 0; i < frame.num_dev; i++)
            {
                DeviceIndices.Add((int)*(frame.dev_idxs + i));
            }
        }

        ~Frame()
        {
            // Release unmanged memory whenver GC finds time I guess
            oepcie.destroy_frame(frame_mem);
        }

        public ulong Time() { return frame.clock; }

        public bool Corrupt() { return frame.corrupt != 0; }

        // NB: This seems horribly inefficient, but I really have no idea

        public T[] Data<T>(int dev_idx) where T : struct
        {
            // Device position in frame
            var pos = DeviceIndices.FindIndex(x => x == dev_idx);

            // If device is not in frame
            if (pos == -1)
            {
                throw new OEException((int)oepcie.Error.DEVIDX);
            }

            // Get the read size and offset for this device
            var num_bytes = DeviceMap[dev_idx].read_size;
            var byte_offset = *(frame.dev_offs + pos);

            var buffer = new byte[num_bytes];
            var output = new T[num_bytes / Marshal.SizeOf(default(T))];
            var start_ptr = frame.data + byte_offset;
            // TODO: Seems like we should be able to copy directly into output!
            Marshal.Copy((IntPtr)start_ptr, buffer, 0, (int)num_bytes);
            Buffer.BlockCopy(buffer, 0, output, 0, (int)num_bytes);
            return output;
        }

        void IDisposable.Dispose()
        {
            oepcie.destroy_frame(frame_mem);
        }

        // Devices with data in this frame
        public List<int> DeviceIndices {get; private set;}

        // Global device index -> device_t struct
        private Dictionary<int, oe.lib.oepcie.device_t> DeviceMap;
        private oepcie.frame_t frame;
        private IntPtr frame_mem;
    }
}
