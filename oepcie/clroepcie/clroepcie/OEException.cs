namespace oe
{
    using System;
    using System.Runtime.Serialization;

    using lib;
    using System.Runtime.InteropServices;

    /// <summary>
    /// An exception thrown by the result of liboepcie.
    /// </summary>
    [Serializable]
    public class OEException : Exception
    {

        public readonly int Number;

        protected OEException()
        { }

        public OEException(int errnum)
        {
            this.Number = errnum;
        }

        public override string ToString()
        {
            return Marshal.PtrToStringAnsi(oepcie.error_str(Number));
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ZException"/> class.
        /// </summary>
        /// <param name="info"><see cref="SerializationInfo"/> that holds the serialized object data about the exception being thrown.</param>
        /// <param name="context"><see cref="StreamingContext"/> that contains contextual information about the source or destination.</param>
        protected OEException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        { }

    }
}