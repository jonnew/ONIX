using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace oni
{
    // Make managed version of oni_frame_t
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct frame_t
    {
        public readonly uint dev_idx; // Array of device indices in frame
        public readonly uint data_sz; // Size in bytes of data buffer
        public readonly byte* data; // Multi-device raw data block
    }

    public unsafe class Frame : SafeHandleZeroOrMinusOneIsInvalid
    {
        protected Frame() 
        : base(true)
        {
        }

        // Ideally, I would like this to be a "Span" into the exsting, allocated frame
        // Now, there are _two_ deep copies happening here as far as I can tell which is ridiculous
        public T[] Data<T>() where T : struct
        {
            var frame = (frame_t*)handle.ToPointer();

            // Get the read size and offset for this device
            var num_bytes = frame->data_sz;

            var buffer = new byte[num_bytes];
            var output = new T[num_bytes / Marshal.SizeOf(default(T))];

            // TODO: Seems like we should be able to copy directly into output or not copy at all!
            Marshal.Copy((IntPtr)frame->data, buffer, 0, (int)num_bytes);
            Buffer.BlockCopy(buffer, 0, output, 0, (int)num_bytes);
            return output;
        }

        // Same as Data() method, this has two copies per call which is ridiculous
        internal void SetData<T>(T[] data) where T : struct
        {
            var frame = (frame_t*)handle.ToPointer();

            // Get the read size and offset for this device
            var num_bytes = frame->data_sz;

            var buffer = new byte[num_bytes];
            Buffer.BlockCopy(data, 0, buffer, 0, (int)num_bytes);
            Marshal.Copy(buffer, 0, (IntPtr)frame->data, (int)num_bytes);
        }

        protected override bool ReleaseHandle()
        {
            lib.NativeMethods.oni_destroy_frame(handle);
            return true;
        }

        // Devices with data in this frame
        public int DeviceIndex()
        {
            return (int)(((frame_t*)handle.ToPointer())->dev_idx);
        }
    }
}
