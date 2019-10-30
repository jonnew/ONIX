#pragma once

#include <onidevices.h>

namespace oni {

    inline const char *device_str(int dev_id)
    {
        return oni_device_str(dev_id);
    }
}
