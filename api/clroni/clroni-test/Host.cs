using System;
using System.Threading;
using System.Runtime.InteropServices;
using oni;

namespace clroni_test
{
    class DataProcessor
    {
        private oni.Context ctx;

        public bool display = false;
        public volatile bool quit = false;
        const int display_downsample = 101;

        public DataProcessor(oni.Context ctx)
        {
            this.ctx = ctx;
        }

        public void CaptureData()
        {
            ulong counter = 0;
           // 
           // int rc = 0;
           // while (rc == 0 && !quit)
           // {
           //     ctx.Write(8, (uint)counter);
           //     Thread.SpinWait(100000); // Sleep(1);
           //     counter++;
           // }

            int rc = 0;
            while (rc == 0 && !quit)
            {
                try
                {
                    var frame = ctx.ReadFrame();

                    if (counter++ % display_downsample == 0)
                    {
                        if (display)
                        {
                            var dat = frame.Data<ushort>();
                            var idx = frame.DeviceIndex();
                            Console.WriteLine("\tDev: {0} ({1})", idx, Device.Name(ctx.DeviceTable[idx].id));
                            Console.WriteLine("\t[{0}]", String.Join(", ", dat));
                        }
                    }

                }
                catch (ONIException ex)
                {
                    Console.Error.WriteLine("Host failed with the following error: "
                                            + ex.ToString());
                    break;
                }
            }
        }
    }

    class Host
    {
        static void Main(string[] args)
        {
            int hw_idx = 0;
            string driver = "";
            if (args.Length == 0 || args.Length > 2) {
                Console.Error.WriteLine("usage:");
                Console.Error.WriteLine("host driver [index]");
                return;
            } else if (args.Length == 1){
                driver = args[0];
            } else if (args.Length == 2) {
                driver = args[0];
                hw_idx = Int32.Parse(args[1]);
            }

            // Get version
            var ver = oni.lib.NativeMethods.LibraryVersion;
            Console.WriteLine("Using liboni version: " + ver);
            bool running = true;

            try
            {

                using(var ctx = new oni.Context(driver, hw_idx))
                {
                    Console.WriteLine("Found the following devices:");
                    foreach (var elem in ctx.DeviceTable.Values) {

                        Console.WriteLine("\t{0}) ID: {1}, Read size: {2}",
                                          elem.idx,
                                          elem.id,
                                          elem.read_size);
                    }

                    // See how big max frames are
                    Console.WriteLine("Max read frame size: "
                                      + ctx.MaxReadFrameSize);

                    // See the hardware clock
                    Console.WriteLine("System clock frequency: "
                                      + ctx.SystemClockHz);

                    // Start acquisition
                    ctx.SetBlockReadSize(4096);
                    ctx.Start();

                    // Start processor in background
                    var processor = new DataProcessor(ctx);
                    var proc_thread
                        = new Thread(new ThreadStart(processor.CaptureData));
                    proc_thread.Start();

                    int c = 's';
                    while (c != 'q') {
                        Console.WriteLine("Enter a command and press enter:");
                        Console.WriteLine("\tc - toggle 1/100 clock display");
                        Console.WriteLine("\td - toggle 1/100 sample display");
                        Console.WriteLine("\tp - toggle stream pause");
                        Console.WriteLine("\tr - enter register command");
                        Console.WriteLine("\tq - quit");
                        Console.Write(">>> ");

                        var cmd = Console.ReadLine();
                        c = cmd[0];

                        if (c == 'p') {
                            running = !running;
                            if (running) {
                                ctx.Start();
                            } else {
                                ctx.Stop();
                                Console.WriteLine("\tPuased.");
                            }
                        } else if (c == 'd') {
                            processor.display = !processor.display;
                        }
                        // else if (c == 'r') {

                        //    printf("Enter dev_idx reg_addr reg_val\n");
                        //    printf(">>> ");

                        //    // Read the command
                        //    char *buf = NULL;
                        //    size_t len = 0;
                        //    rc = getline(&buf, &len, stdin);
                        //    if (rc == -1) { printf("Error: bad command\n");
                        //    continue;}

                        //    // Parse the command string
                        //    long values[3];
                        //    rc = parse_reg_cmd(buf, values);
                        //    if (rc == -1) { printf("Error: bad command\n");
                        //    continue;}
                        //    free(buf);

                        //    size_t dev_idx = (size_t)values[0];
                        //    oni_reg_addr_t addr = (oni_reg_addr_t)values[1];
                        //    oni_reg_val_t val = (oni_reg_val_t)values[2];

                        //    oni_write_reg(ctx, dev_idx, addr, val);
                        //}
                    }

                    // Stop hardware
                    //ctx.Stop();

                    // Join data and signal threads
                    processor.quit = true;
                    proc_thread.Join(200);

                } // ctx.Dispose() is called.

            } catch (ONIException ex) {
                Console.Error.WriteLine("Host failed with the following error: "
                                        + ex.ToString());
                Console.Error.WriteLine("Current errno: "
                                        + Marshal.GetLastWin32Error());
            }
        }
    }
}
