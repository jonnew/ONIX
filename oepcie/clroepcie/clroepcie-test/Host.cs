using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;
using System.Threading;
using oe;
using System.Runtime.InteropServices;

namespace clroepcie_test
{
    class DataProcessor
    {
        oe.Context ctx;
        bool quit = false;
        bool display = false;
        bool display_clock = true;

        public DataProcessor(oe.Context ctx)
        {
            this.ctx = ctx;
        }

        public void CaptureData()
        {
            int rc = 0;
            while (rc == 0 && !quit)  {

                var frame = ctx.ReadFrame();

                if (display_clock)
                    System.Console.WriteLine("\tSample: {0}\n", frame.Time());
            }
        }

    }

    class Host
    {
        static void Main(string[] args)
        {
            // Ger version
            var ver = oe.lib.oepcie.LibraryVersion;
            System.Console.WriteLine("Using liboepcie version: " + ver);

            try
            {
                // Open context
                using (var ctx = new oe.Context("/tmp/rat128_config",
                                                "/tmp/rat128_read",
                                                "/tmp/rat128_signal"))
                {

                    System.Console.WriteLine("Found the following devices:" );
                    foreach (var elem in ctx.DeviceMap) {

                        var index = elem.Item1;
                        var device = elem.Item2;

                        System.Console.WriteLine("\t{0}) ID: {1}, Read size: {2}",
                        index,
                        device.id,
                        device.read_size);
                    }

                    // Start acqusisition
                    ctx.Start();

                    // See how big max frame is
                    int frame_size = ctx.GetOption(Context.Option.MAXREADFRAMESIZE);
                    System.Console.WriteLine("Max read frame size: " + frame_size);

                    // See if we are running
                    int hz = ctx.GetOption(Context.Option.SYSCLKHZ);
                    System.Console.WriteLine("System clock frequency: " + hz);

                    // See if we are running
                    int running = ctx.GetOption(Context.Option.RUNNING);
                    System.Console.WriteLine("Running state: " + running);

                    // Start processor in background
                    var processor = new DataProcessor(ctx);
                    var proc_thread = new Thread(new ThreadStart(processor.CaptureData));
                    proc_thread.Start();
                    proc_thread.Join();

                } // ctx.Dispose() is called.

            }
            catch (OEException ex)
            {
                System.Console.Error.WriteLine("liboepcie failed with the following error: " + ex.ToString());
                System.Console.Error.WriteLine("Current errno: " + Marshal.GetLastWin32Error());
            }
        }
    }
}
