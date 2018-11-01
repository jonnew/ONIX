// I'm not using e.g. NUnit here because its a pain in the ass to install and
// probably overkill for testing bindings
namespace oe
{
    using System;
    using oe.lib;

    public class Test
    {
        static public void Main()
        {

            printPassFail(liboepcieTest(), "liboepcie test");
            printPassFail(ErrorTest(), "Error test");

        }

        static void printPassFail(bool pass, String test_name)
        {
            if (pass) {
                Console.WriteLine("PASS: " + test_name );
            } else {
                Console.WriteLine("FAIL: " + test_name );
            }
        }

        static bool liboepcieTest(bool print = true)
        {
            // oe_create_ctx
            var ctx = oepcie.create_ctx();

            if (IntPtr.Zero.Equals(ctx))
                return false;

            if (print)
                Console.WriteLine("\t liboepcie version: " + oepcie.LibraryVersion);

            // oe_init_ctx
            var rc = oepcie.init_ctx(ctx);
            Console.WriteLine("\t oepcie.init_ctx(): " + rc);

            return true;
        }

        static bool ErrorTest(bool print = true)
        {
            //Console.WriteLine("\tSuccess:" + Error.Success.ToString);
            return true;
        }


   }
}
