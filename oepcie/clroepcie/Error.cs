namespace oe
{
	using System;
	using System.Collections.Generic;
	using System.Linq;
	using System.Reflection;
	using System.Runtime.InteropServices;
	using lib;

    public class Error
	{
		internal Error(int errno)
		{
			this.num = errno;
		}

        private int num;
        public int Number { get { return num; } }

        public string Name
        {
            get
            {
                string result;
                return oeErrorToName.TryGetValue(this, out result) ? result : "<unknown>";
            }
        }

        public string Text {
            get {
                return Marshal.PtrToStringAnsi(oepcie.error_str(num));
            }
        }

		private static void PickupErrors(ref IDictionary<Error, string> errors)
		{

            Type type = typeof(Error);
			FieldInfo[] fields = type.GetFields(BindingFlags.Static | BindingFlags.Public);

			Type codeType = type.GetNestedType("Code", BindingFlags.NonPublic);

			foreach (FieldInfo symbolField in fields.Where(f => typeof(Error).IsAssignableFrom(f.FieldType)))
			{
				FieldInfo symbolCodeField = codeType.GetField(symbolField.Name);
				if (symbolCodeField != null)
				{
					int symbolNumber = (int)symbolCodeField.GetValue(null);

				    var symbol = Activator.CreateInstance(
				        type,
				        BindingFlags.NonPublic | BindingFlags.Instance,
				        null,
				        new object[] {symbolNumber},
				        null);
					symbolField.SetValue(null, symbol);
					errors.Add((Error)symbol, symbolCodeField.Name);
				}
			}
		}

		//public static readonly Error None = default(Error);

		static Error()
		{
			IDictionary<Error, string> errors = new Dictionary<Error, string>();
			PickupErrors(ref errors);
			oeErrorToName = errors;
            //var one = Error.None;
		}

		static IDictionary<Error, string> oeErrorToName;

		public static IEnumerable<Error> Find(string symbol)
		{
			return oeErrorToName
				.Where(s => s.Value != null && (s.Value == symbol)).Select(x => x.Key);
		}

		public static IEnumerable<Error> Find(string ns, int num)
		{
			return oeErrorToName
				.Where(s => s.Value != null && (s.Value.StartsWith(ns) && s.Key.num == num)).Select(x => x.Key);
		}

		public override bool Equals(object obj)
		{
			return Error.Equals(this, obj);
		}

	    public new static bool Equals(object a, object b)
	    {
	        if (object.ReferenceEquals(a, b))
	        {
	            return true;
	        }

	        var symbolA = a as Error;
	        var symbolB = b as Error;

	        return symbolA != null && symbolB != null && symbolA.num == symbolB.num;
	    }

		public override int GetHashCode()
		{
			return Number.GetHashCode();
		}

		public override string ToString()
		{
			return Name + "(" + Number + "): " + Text;
		}

		public static implicit operator int(Error errnum)
		{
			return errnum.Number;
		}

        //static Error()
        //{
        //}

        internal static class Code
		{
            // TODO: find a way to make this independent of the Windows SDK
            // version that libzmq was built against TODO: are all of these
            // actually used by libzmq?  these values are the Windows error
            // codes as defined by the Windows 10 SDK when
            // _CRT_NO_POSIX_ERROR_CODES is not defined
            public static readonly int
                OE_ESUCCESS         =  0,  // Success
                OE_EPATHINVALID     = -1,  // Invalid stream path, fail on open
                OE_EREINITCTX       = -2,  // Double initialization attempt
                OE_EDEVID           = -3,  // Invalid device ID on init or reg op
                OE_EREADFAILURE     = -4,  // Failure to read from a stream/register
                OE_EWRITEFAILURE    = -5,  // Failure to write to a stream/register
                OE_ENULLCTX         = -6,  // Attempt to call function w null ctx
                OE_ESEEKFAILURE     = -7,  // Failure to seek on stream
                OE_EINVALSTATE      = -8,  // Invalid operation for the current context run state
                OE_EDEVIDX          = -9,  // Invalid device index
                OE_EINVALOPT        = -10, // Invalid context option
                OE_EINVALARG        = -11, // Invalid function arguments
                OE_ECANTSETOPT      = -12, // Option cannot be set in current context state
                OE_ECOBSPACK        = -13, // Invalid COBS packet
                OE_ERETRIG          = -14, // Attempt to trigger an already triggered operation
                OE_EBUFFERSIZE      = -15, // Supplied buffer is too small
                OE_EBADDEVMAP       = -16, // Badly formated device map supplied by firmware
                OE_EBADALLOC        = -17, // Bad dynamic memory allocation
                OE_ECLOSEFAIL       = -18, // File descriptor close failure, check errno
                OE_EDATATYPE        = -19, // Invalid underlying data types
                OE_EREADONLY        = -20, // Attempted write to read only object (register, context option, etc)
                OE_ERUNSTATESYNC    = -21  // Software and hardware run state out of sync
			;
		}

		public static Error FromErrno(int num)
		{
            // TODO: this can be made more efficient
			Error symbol = Find("E", num).OfType<Error>().FirstOrDefault();
			if (symbol != null) return symbol;

            // unexpected error
			return new Error(num);
		}

		public static Error None
		{
			get
			{
				return default(Error); // null
			}
		}

		public static readonly Error
			// DEFAULT = new ZmqError(0),
            OE_ESUCCESS         ,
            OE_EPATHINVALID     ,
            OE_EREINITCTX       ,
            OE_EDEVID           ,
            OE_EREADFAILURE     ,
            OE_EWRITEFAILURE    ,
            OE_ENULLCTX         ,
            OE_ESEEKFAILURE     ,
            OE_EINVALSTATE      ,
            OE_EDEVIDX          ,
            OE_EINVALOPT        ,
            OE_EINVALARG        ,
            OE_ECANTSETOPT      ,
            OE_ECOBSPACK        ,
            OE_ERETRIG          ,
            OE_EBUFFERSIZE      ,
            OE_EBADDEVMAP       ,
            OE_EBADALLOC        ,
            OE_ECLOSEFAIL       ,
            OE_EDATATYPE        ,
            OE_EREADONLY        ,
            OE_ERUNSTATESYNC 
		;
	}
}
