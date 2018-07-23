using System;
using System.Threading;
using System.Runtime.InteropServices;
using oe;


namespace clroepcie_test
{


    class DataProcessor
{
    private oe.Context ctx;

    public bool display = false;
    public bool display_clock = false;
    public volatile bool quit = false;
    const int display_downsample = 100;
    ulong counter = 0;

    public DataProcessor(oe.Context ctx)
    {
        this.ctx = ctx;
    }

    public void CaptureData()
    {
        int rc = 0;
        while (rc == 0 && !quit)  {

            var frame = ctx.ReadFrame();

                if (counter++ % display_downsample == 0)
                {
                    if (display_clock)
                        Console.WriteLine("\tSample: {0}\n", frame.Time());

                    if (display)
                    {
                        foreach (var idx in frame.DeviceIndices)
                        {
                            var dat = frame.Data<ushort>(idx);
                            Console.WriteLine("\tDev: {0} ({1})", idx, Device.Name((int)ctx.DeviceMap[idx].id));
                            Console.WriteLine("\t[{0}]", String.Join(", ", dat));
                        }
                    }
                }
            }
        }
    }

class Host
{
    static void Main(string[] args)
    {
        // Get version
        var ver = oe.lib.NativeMethods.LibraryVersion;
        Console.WriteLine("Using liboepcie version: " + ver);
        bool running = true;

        try
        {
                // Open context
                /*using (var ctx = new oe.Context("/tmp/rat128_config",
                                                "/tmp/rat128_read",
                                                "/tmp/rat128_signal"))*/
                using (var ctx = new oe.Context())
                {

                    Console.WriteLine("Found the following devices:");
                    foreach (var elem in ctx.DeviceMap)
                    {
                        var index = elem.Key;
                        var device = elem.Value;

                        Console.WriteLine("\t{0}) ID: {1}, Read size: {2}",
                        index,
                        device.id,
                        device.read_size);
                    }

                    // See how big max frames are
                    Console.WriteLine("Max read frame size: " + ctx.MaxReadFrameSize);
                    Console.WriteLine("Max write frame size: " + ctx.MaxWriteFrameSize);

                    // See the hardware clock
                    Console.WriteLine("System clock frequency: " + ctx.SystemClockHz);

                    // Start acqusisition
                    ctx.Start();

                    // Start processor in background
                    var processor = new DataProcessor(ctx);
                    var proc_thread = new Thread(new ThreadStart(processor.CaptureData));
                    proc_thread.Start();

                    int c = 's';
                    while (c != 'q')
                    {
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
                            if (running)
                            {
                                ctx.Start();
                            }
                            else
                            {
                                ctx.Stop();
                                Console.WriteLine("\tPuased.");
                            }
                        } else if (c == 'c') {
                            processor.display_clock = !processor.display_clock;
                        }
                        else if (c == 'd')
                        {
                            processor.display = !processor.display;
                        }
                        //else if (c == 'r') {

                        //    printf("Enter dev_idx reg_addr reg_val\n");
                        //    printf(">>> ");

                        //    // Read the command
                        //    char *buf = NULL;
                        //    size_t len = 0;
                        //    rc = getline(&buf, &len, stdin);
                        //    if (rc == -1) { printf("Error: bad command\n"); continue;}

                        //    // Parse the command string
                        //    long values[3];
                        //    rc = parse_reg_cmd(buf, values);
                        //    if (rc == -1) { printf("Error: bad command\n"); continue;}
                        //    free(buf);

                        //    size_t dev_idx = (size_t)values[0];
                        //    oe_reg_addr_t addr = (oe_reg_addr_t)values[1];
                        //    oe_reg_val_t val = (oe_reg_val_t)values[2];

                        //    oe_write_reg(ctx, dev_idx, addr, val);
                        //}
                    }

                    // Join data and signal threads
                    processor.quit = true;

                    // Stop hardware, join data collection thread, and reset
                    ctx.Stop();
                    proc_thread.Join();
                    ctx.Reset();

                } // ctx.Dispose() is called.

        }
        catch (OEException ex)
        {
            Console.Error.WriteLine("liboepcie failed with the following error: " + ex.ToString());
            Console.Error.WriteLine("Current errno: " + Marshal.GetLastWin32Error());
        }
    }
}
}
