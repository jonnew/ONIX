namespace oe
{
    using System;
    using System.Runtime.Serialization;

    using lib;
    using System.Runtime.InteropServices;

    [Serializable]
    public class OEException : Exception
    {

        public readonly int Number;

        protected OEException()
        {

        }

        public OEException(int errnum)
        {
            this.Number = errnum;
        }

        public override string ToString()
        {
            return Marshal.PtrToStringAnsi(NativeMethods.oe_error_str(Number));
        }

        protected OEException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        { }

    }
}
