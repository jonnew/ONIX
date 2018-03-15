using System;
using System.Collections.Generic;
using MapDevice = System.Tuple<int, oe.lib.oepcie.device_t>;

namespace oe
{
    using lib;
    using System.Runtime.InteropServices;

    public unsafe class Frame : IDisposable
    {
        public Frame(List<MapDevice> dev_map, IntPtr frame_mem)
        {
            // Deep copy of frame memroy into managed memory
            this.frame_mem = frame_mem;
            frame = (oepcie.frame_t)Marshal.PtrToStructure(frame_mem, typeof(oepcie.frame_t));

            // Create frame-specific device map
            DeviceMap = new Dictionary<uint, MapDevice>(); // (frame.num_dev);
            for (int i = 0; i < frame.num_dev; i++)
            {
                DeviceMap.Add(*(frame.dev_idxs + i), new MapDevice(i, dev_map[(int)*(frame.dev_idxs + i)].Item2));
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
        public byte[] Data(uint dev_idx)
        {
            if (!DeviceMap.ContainsKey(dev_idx))
            {
                throw new OEException((int)oepcie.Error.DEVIDX); 
            }

            // Get the byte size of the array
            var num_bytes = DeviceMap[dev_idx].Item2.read_size;
            var byte_offset = *(frame.dev_offs + DeviceMap[dev_idx].Item1);          

            var output = new byte[num_bytes];
            int j = 0;
            var start_ptr = frame.data + byte_offset;
            Marshal.Copy((IntPtr)start_ptr, output, 0, (int)num_bytes);
            return output;
        }

        void IDisposable.Dispose()
        {
            oepcie.destroy_frame(frame_mem);
        }

        // Global device index -> device_t struct
        public readonly Dictionary<uint, MapDevice> DeviceMap; 
        private oepcie.frame_t frame;
        private IntPtr frame_mem;
    }
}