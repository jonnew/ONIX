namespace oni
{
    using System;
    using System.Runtime.InteropServices;
    using System.Runtime.Serialization;

    using lib;

    [Serializable]
    public class ONIException : Exception
    {

        public readonly int Number;

        protected ONIException()
        {

        }

        public ONIException(int errnum)
        {
            this.Number = errnum;
        }

        public override string ToString()
        {
            return Marshal.PtrToStringAnsi(NativeMethods.oni_error_str(Number));
        }

        protected ONIException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        { }

    }
}
