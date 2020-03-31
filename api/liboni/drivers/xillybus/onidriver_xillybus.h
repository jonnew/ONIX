#ifndef __LIBONI_DRIVER_XILLYBUS_H__
#define __LIBONI_DRIVER_XILLYBUS_H__

// Driver options
enum  {
    ONI_XILLYBUS_CONFIGSTREAMPATH,
    ONI_XILLYBUS_SIGNALSTREAMPATH,
    ONI_XILLYBUS_READSTREAMPATH,
    ONI_XILLYBUS_WRITESTREAMPATH
};

// Default paths
#ifdef _WIN32
#define ONI_XILLYBUS_DEFAULTCONFIGPATH  "\\\\.\\xillybus_oni_config_32"
#define ONI_XILLYBUS_DEFAULTREADPATH    "\\\\.\\xillybus_oni_data_input_32"
#define ONI_XILLYBUS_DEFAULTWRITEPATH   "\\\\.\\xillybus_oni_data_output_32"
#define ONI_XILLYBUS_DEFAULTSIGNALPATH  "\\\\.\\xillybus_oni_signal_8"
#else
#define ONI_XILLYBUS_DEFAULTCONFIGPATH  "/dev/xillybus_oni_config_32"
#define ONI_XILLYBUS_DEFAULTREADPATH    "/dev/xillybus_oni_data_input_32"
#define ONI_XILLYBUS_DEFAULTWRITEPATH   "/dev/xillybus_oni_data_output_32"
#define ONI_XILLYBUS_DEFAULTSIGNALPATH  "/dev/xillybus_oni_signal_8"
#endif

#define XILLYBUS_DRIVER_NAME "xillybus"

#endif
