using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace oe
{
    public class Device
    {

        public Device(IntPtr device)
        {
            device_ = device;
        }

        private IntPtr device_;
    }
}
