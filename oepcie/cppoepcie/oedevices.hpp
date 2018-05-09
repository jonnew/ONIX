#pragma once

#include <oedevices.h>

namespace oe {

    inline bool device_official(int dev_id)
    {
        return (oe_device_official(dev_id) == 0);
    }

    inline const char *device_str(int dev_id)
    {
        return oe_device_str(dev_id);
    }
}
