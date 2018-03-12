using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using oe;
using System.Runtime.InteropServices;

namespace clroepcie_test
{
    class Program
    {
        static void Main(string[] args)
        {
            // Ger version
            var ver = oe.lib.oepcie.LibraryVersion;
            System.Console.WriteLine("Using liboepcie version: " + ver);

            // Open context
            try
            {
                using (var ctx = new oe.Context())
                {
                    // Start acqusisition
                    ctx.SetOption(Context.Option.RUNNING, 1);

                    // See how big max frame is
                    int frame_size = ctx.GetOption(Context.Option.READFRAMESIZE);
                    System.Console.WriteLine("Frame size: " + frame_size);

                    // See if we are running
                    int hz = ctx.GetOption(Context.Option.SYSCLKHZ);
                    System.Console.WriteLine("System clock frequency: " + hz);

                    // See if we are running
                    int running = ctx.GetOption(Context.Option.RUNNING);
                    System.Console.WriteLine("Running state: " + running);

                    var frame = ctx.ReadFrame();

                    if (frame.contains(0) != -1) {
                        var dat = frame.data<UInt16>(0);
                    }


                } // ctx.Dispose() is called.

            }
            catch (OEException ex)
            {
                System.Console.Error.WriteLine("liboepcie failed with the following error: " + ex.ToString());
                System.Console.Error.WriteLine("Current errno" + Marshal.GetLastWin32Error());
            }
        }
    }
}
