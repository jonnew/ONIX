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
            var ver = oe.lib.oepcie.LibraryVersion;

            var ctx = new oe.Context();

            System.Console.WriteLine(ver);

            //var context = new oe.Context();
        }
    }
}
