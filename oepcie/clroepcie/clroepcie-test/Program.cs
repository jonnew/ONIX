using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using oe;

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
                var ctx = new oe.Context();
            }
            catch (OEException ex)
            {
                System.Console.Error.WriteLine("liboepcie failed with the following error: " + ex.ToString());
            }
        }
    }
}
