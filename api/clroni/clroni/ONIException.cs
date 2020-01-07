using System;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;

namespace oni
{
    [Serializable]
    public class ONIException : Exception
    {
        public readonly lib.Error Number;

        protected ONIException() { }

        public ONIException(int errnum)
        {
            Number = (lib.Error)errnum;
        }

        public ONIException(lib.Error errnum)
        {
            Number = errnum;
        }

        public override string ToString()
        {
            return Marshal.PtrToStringAnsi(lib.NativeMethods.oni_error_str((int)Number));
        }

        public override string Message
        {
            get
            {
                return ToString();
            }
        }

        protected ONIException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        { }
    }
}
