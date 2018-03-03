using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace oe
{
    using DeviceMap = List<Device>;

    public class Frame
    {

        public Frame(ref DeviceMap dev_map) {
            this.dev_map = dev_map;
        }

        // TODO: In C++, this is a reference to the device map in context. Here it is copied I think.
        public readonly DeviceMap dev_map;
    }
}
