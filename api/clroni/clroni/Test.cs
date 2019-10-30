// I'm not using e.g. NUnit here because its a pain in the ass to install and
// probably overkill for testing bindings
namespace oni
{
    using System;
    using oni.lib;

    public class Test
    {
        static public void Main()
        {

            printPassFail(liboniTest(), "liboni test");
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

        static bool liboniTest(bool print = true)
        {
            // oni_create_ctx
            var ctx = oni.create_ctx();

            if (IntPtr.Zero.Equals(ctx))
                return false;

            if (print)
                Console.WriteLine("\t liboni version: " + oni.LibraryVersion);

            // oni_init_ctx
            var rc = oni.init_ctx(ctx);
            Console.WriteLine("\t oni.init_ctx(): " + rc);

            return true;
        }

        static bool ErrorTest(bool print = true)
        {
            //Console.WriteLine("\tSuccess:" + Error.Success.ToString);
            return true;
        }


   }
}
