#pragma once

#include <oedevices.h>

namespace oe {

    inline const char *device_str(int dev_id)
    {
        return oe_device_str(dev_id);
    }
}
