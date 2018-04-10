// This was taken directly from clrzmq4

namespace oe.lib
{
    using System;
    using System.Text;
    using System.Runtime.InteropServices;

    internal sealed partial class DispoIntPtr : IDisposable
    {
        private static DispoIntPtr AllocStringNative(string str, out int byteCount)
        {
            // use builtin allocation
            var dispPtr = new DispoIntPtr();
            dispPtr._ptr = Marshal.StringToHGlobalAnsi(str);
            dispPtr.is_allocated = true;

            byteCount = Encoding.Default.GetByteCount(str);
            //byteCount = ASCIIEncoding.ASCII.GetByteCount(str);
            //System.Console.WriteLine("{0}: {1} bytes.", str, byteCount);
            return dispPtr;
        }

        public static DispoIntPtr Alloc(int size)
        {
            var dispPtr = new DispoIntPtr();
            dispPtr._ptr = Marshal.AllocHGlobal(size);
            dispPtr.is_allocated = true;
            return dispPtr;
        }

        public static DispoIntPtr AllocString(string str)
        {
            int byteCount;
            return AllocStringNative(str, out byteCount);
        }

        public static DispoIntPtr AllocString(string str, out int byteCount)
        {
            return AllocStringNative(str, out byteCount);
        }

        public static implicit operator IntPtr(DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? IntPtr.Zero : dispoIntPtr._ptr;
        }

        unsafe public static explicit operator void* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (void*)null : (void*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator byte* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (byte*)null : (byte*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator sbyte* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (sbyte*)null : (sbyte*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator short* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (short*)null : (short*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator ushort* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (ushort*)null : (ushort*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator char* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (char*)null : (char*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator int* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (int*)null : (int*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator uint* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (uint*)null : (uint*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator long* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (long*)null : (long*)dispoIntPtr._ptr;
        }

        unsafe public static explicit operator ulong* (DispoIntPtr dispoIntPtr)
        {
            return dispoIntPtr == null ? (ulong*)null : (ulong*)dispoIntPtr._ptr;
        }

        private bool is_allocated;

        private IntPtr _ptr;

        public IntPtr Ptr
        {
            get { return _ptr; }
        }

        internal DispoIntPtr() { }

        ~DispoIntPtr()
        {
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        void Dispose(bool disposing)
        {
            // TODO: instance ThreadStatic && do ( o == null ? return : ( lock(o, ms), check threadId, .. ) )
            IntPtr handle = _ptr;
            if (handle != IntPtr.Zero)
            {
                if (is_allocated)
                {
                    Marshal.FreeHGlobal(handle);
                    is_allocated = false;
                }
                _ptr = IntPtr.Zero;
            }
        }

        /* public void ReAlloc(long size) {
			_ptr = Marshal.ReAllocHGlobal(_ptr, new IntPtr(size));
		} */
    }
}
