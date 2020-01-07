# `liboni`
ANSI C implementation of the [Open Neuro Interface API
Specification](https://github.com/jonnew/ONI).

## Scope and External Dependencies
`liboni` is a C library that implements the [Open Ephys++ API
Specification](#api-spec). It is written in C to facilitate
cross platform and cross-language use. It is composed of the following files:

1. oni.h and oni.c: main API implementation
1. onidevice.h and onidevices.c: officially supported device and register
   definitions. This file can be ignored for project that do not wish to
   conform to the official device specification.
1. ondriverloader.h and onidriverloader.c: functions for dynamically loading
   hardware translation driver.
1. onidriver.h: hardware translation driver header that must be implemented for
   a particular host hardware

`liboni` is a low level library used by high-level language binding and/or
software plugin developers. It is not meant to be used by neuroscientists
directly. The only external dependency aside from the C standard library and
dynamic library loading functions is is a hardware translation driver ("driver") that

fulfills the requirements of the ONI Host Interconnect Specification
Specification](#comm-protocol). This implementation contains drivers for

1. [Xillybus](http://xillybus.com/):provides proprietary FPGA IP cores and
free and open source device drivers to allow the communication channels to be
implemented using the PCIe bus.
1. RIFFA: TODO
1. FTDI USB3.0: TODO
1. Opal-Kelly USB3.0: TODO

From the API's perspective, hardware communication abstracted to IO system
calls (`open`, `read`, `write`, etc.) on file descriptors. File descriptor
semantics and behavior are identical to either normal files (configuration
channel) or named pipes (signal, data input, and data output channels).
Importantly, the low-level synchronization, resource allocation, and logic
required to use the hardware communication backend is implicit to `liboni` API
function calls. Orchestration of the communication backend _is not directly
managed by the library user_.

## License
[MIT](https://en.wikipedia.org/wiki/MIT_License)

## Build

### Linux
For build options, look at the [Makefile](Makefile). To build and install:
```
$ make <options>
$ make install PREFIX=/path/to/install
```
to place headers in whatever path is specified by PREFIX. PREFIX defaults to
`/usr/lib/include`. You can uninstall (delete headers and libraries) via
```
$ make uninstall PREFIX=/path/to/uninstall
```
To make a particular driver, navigate to its location within the `drivers`
subdirectory and:
```
$ make <options>
$ make install PREFIX=/path/to/install
```
Then update dynamic library cache via:
```
$ ldconfig
```

### Windows
Open the included Visual Studio solution and press play. For whatever reason,
it seems that the startup project is not consistently saved with the solution.
So make sure that is set to `liboni-test` in the solution properties.

## Test Programs (Linux Only)
The [liboni-test](liboni-test) directory contains minimal working programs that use this library.

1. `firmware` : Emulate hardware. Stream fake data over UNIX pipes (Linux only)
1. `host` : Basic data acquisition loop. Communicate with `firmware` or actual
   hardware (Linux and Windows).

## Performance Testing (Linux Only)
1. Install google perftools:
```
$ sudo apt-get install google-perftools
```
2. Check what library is installed:
```
ldconfig -p | grep profiler
```
if `libprofiler.so` is not there, but `libprofiler.so.x` exists, create a softlink:
```
sudo ln -rs /path/to/libprofiler.so.x /path/to/libprofiler.so
```
3. Link test programs against the CPU profiler:
```
$ cd liboni-test
$ make profile
```
4. Run the `firmware` program to serve fake data. Provide a numerical argument
   specifying the number of fake frames to produce. It will tell you how long
   it takes `host` to sink all these frames. This is host processing time +
   UNIX pipe read/write.
```
$ cd bin
$ ./firmware 10e6
```
5. Run the `host` program while dumping profile info:
```
$ env CPUPROFILE=/tmp/host.prof ./host /tmp/xillybus_cmd_32 /tmp/xillybus_signal_8 /tmp/xillybus_data_read_32
```
6. Examine output
```
$ pprof ./host /tmp/host.prof
```

## Memory Testing (Linux Only)
Run the `host` program with valgrind using full leak check
```
$ valgrind --leak-check=full ./host /tmp/xillybus_cmd_32 /tmp/xillybus_signal_8 /tmp/xillybus_data_read_32
```

# API Reference

## Types

### Integer types
- `oni_size_t`: Fixed width size integer type.
- `oni_dev_id_t`: Fixed width device identity integer type.
- `oni_reg_addr_t`: Fixed width device register address integer type.
- `oni_reg_value_t`: Fixed width device register value integer type.

### `oni_ctx`
[Context](#context) implementation. `oni_ctx` is an opaque handle to a context
structure which contains hardware and device state information.

```
// oepcie.h
typedef struct oni_ctx_impl *oni_ctx;
```

Context details are hidden in implementation file (oepcie.c):

``` {.c}
typedef struct stream_fid {
    char *path;
    int fid;
} stream_fid_t;

typedef struct oni_ctx_impl {

    // Communication channels
    stream_fid_t config;
    stream_fid_t read;
    stream_fid_t write;
    stream_fid_t signal;

    // Devices
    oni_size_t num_dev;
    oni_device_t* dev_map;

    // Maximum frame sizes (bytes)
    oni_size_t max_read_frame_size;
    oni_size_t write_frame_size;

    // Data buffer
    uint8_t *buffer;
    uint8_t *buff_read_pos;
    uint8_t *buff_end_pos;

    // Acqusition state
    enum run_state {
        CTXNULL = 0,
        UNINITIALIZED,
        IDLE,
        RUNNING
    } run_state;

} oni_ctx_impl_t;
```

Each context manages a single device map. Following a hardware reset, which is
triggered either by a call to `oni_init_ctx` or to `oni_set_opt` using the
`OE_RESET` option, the context `run_state` is set to UNINTIALIZED and the device map is
pushed onto the signal stream by the FPGA as
[COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) encode
packets. On the signal stream, the device map is organized as follows,

... | DEVICEMAPACK, uint32_t num_devices | DEVICEINST oni_device_t dev_0
    | DEVICEINST oni_device_t dev_1 | ... | DEVICEINST oni_device_t dev_n | ...

where | represents '0' packet delimiters. During a call to `oni_init_ctx`, the
device map is decoded from the signal stream. It can then be examined using
calls to `oni_get_opt` using the `OE_DEVICEMAP` option. After the map is
received, the context `run_state` becomes IDLE. A call to `oni_set_ctx` with the
`OE_RUNNING` option can then be used to start acquisition by transitioning the
context `run_state` to `RUNNING`.

### `oni_device_t`
[Device](#device) implementation. An `oni_device_t` describes one of potentially
many pieces of hardware within a context. Examples include Intan chips, IMUs,
optical stimulator's, camera sensors, etc. Each valid device type has a unique
ID which is enumerated in the auxiliary `oedevices.h` file or some use-specific
header. A map of available devices is read from hardware and stored in the
current context via a call to [`oni_init_ctx`](#oni_init_ctx).  This map can be
examined via calls to [`oni_get_opt`](#oni_get_opt).

``` {.c}
typedef struct {
    oni_dev_id_t id;         // Device ID number
    oni_size_t slot;         // Device slot
    oni_size_t clock_dom;    // Device clock domain
    oni_size_t clock_hz;     // Clock rate in Hz of clock_dom
    oni_size_t read_size;    // Device data read size per frame in bytes
    oni_size_t num_reads;    // Number of read frames to construct a full sample
    oni_size_t write_size;   // Device data write size per frame in bytes
    oni_size_t num_writes;   // Number of written frames comprising a full sample
} oni_device_t;
```

Officially supported device IDs and configuration register definitions are
provided in oedevices.h as a set of enumerations. A portion of the official
device ID enumeration is defined as follows:

``` {.c}
typedef enum device_id {
    OE_IMMEDIATEIO = 0,
    OE_RHD2132,
    OE_RHD2164,
    OE_MPU9250,
    OE_ESTIM,
    ...
    OE_MAXDEVICEID = 9999
} oni_device_id_t

```

An example of a device register (for the `OE_ESTIM` device ID) enumeration is:

``` {.c}
enum oni_estim_regs {
    OE_ESTIM_NULLPARM    = 0,  // No command
    OE_ESTIM_BIPHASIC    = 1,  // Biphasic pulse (0 = monophasic, 1 = biphasic;
    OE_ESTIM_CURRENT1    = 2,  // Phase 1 current, (0 to 255 = -1.5 mA to +1.5mA)
    OE_ESTIM_CURRENT2    = 3,  // Phase 2 voltage, (0 to 255 = -1.5 mA to +1.5mA)
    OE_ESTIM_PULSEDUR1   = 4,  // Phase 1 duration, 10 microsecond steps
    OE_ESTIM_IPI         = 5,  // Inter-phase interval, 10 microsecond steps
    OE_ESTIM_PULSEDUR2   = 6,  // Phase 2 duration, 10 microsecond steps
    OE_ESTIM_PULSEPERIOD = 7,  // Inter-pulse interval, 10 microsecond steps
    OE_ESTIM_BURSTCOUNT  = 8,  // Burst duration, number of pulses in burst
    OE_ESTIM_IBI         = 9,  // Inter-burst interval, microseconds
    OE_ESTIM_TRAINCOUNT  = 10, // Pulse train duration, number of bursts in train
    OE_ESTIM_TRAINDELAY  = 11, // Pulse train delay, microseconds
    OE_ESTIM_TRIGGER     = 12, // Trigger stimulation (1 = deliver)
    OE_ESTIM_POWERON     = 13, // Control estim sub-circuit power (0 = off, 1 = on)
    OE_ESTIM_ENABLE      = 14, // Control null switch (0 = stim output shorted to ground, 1 = enabled)
    OE_ESTIM_RESTCURR    = 15, // Current between pulse phases, (0 to 255 = -1.5 mA to +1.5mA)
    OE_ESTIM_RESET       = 16, // Reset all parameters to default
};
```

These registers may be familiar to those who have used a Master-8 or
[pulse-pal](https://sites.google.com/site/pulsepalwiki/) stimulus sequencer.

### `oni_frame_t`
[Frame](#frame) implementation. Frames are produced by calls `oni_read_frame`.
Frames are actually zero-copy `views` into an external, RAII-capable circular
buffer (the `buffer` handle). When implementing language bindings, simply
ignore this member's existence.

``` {.c}
typedef struct oni_frame {
    // Header
    uint64_t clock;         // Base clock counter
    uint16_t num_dev;       // Number of devices in frame
    uint8_t corrupt;        // Is this frame corrupt?

    // Data
    oni_size_t *dev_idxs;    // Array of device indices in frame
    oni_size_t *dev_offs;    // Device data offsets within data block
    uint8_t *data;          // Multi-device raw data block
    oni_size_t data_sz;      // Size in bytes of data buffer

    // External buffer, don't touch
    oni_buffer buffer;       // Handle to external buffer

} oni_frame_t;
```

### `oni_opt_t`
Context option enumeration. See the description of `oni_set_opt` and
`oni_get_opt` for valid values.

### `oni_error_t`
Error code enumeration.

``` {.c}
typedef enum oni_error {
    OE_ESUCCESS         =  0,  // Success
    OE_EPATHINVALID     = -1,  // Invalid stream path, fail on open
    OE_EDEVID           = -2,  // Invalid device ID on init or reg op
    OE_EDEVIDX          = -3,  // Invalid device index
    OE_EWRITESIZE       = -4,  // Data write size is incorrect for designated device
    OE_EREADFAILURE     = -5,  // Failure to read from a stream/register
    OE_EWRITEFAILURE    = -6,  // Failure to write to a stream/register
    OE_ENULLCTX         = -7,  // Attempt to call function w null ctx
    OE_ESEEKFAILURE     = -8,  // Failure to seek on stream
    OE_EINVALSTATE      = -9,  // Invalid operation for the current context run state
    OE_EINVALOPT        = -10, // Invalid context option
    OE_EINVALARG        = -11, // Invalid function arguments
    OE_ECOBSPACK        = -12, // Invalid COBS packet
    OE_ERETRIG          = -13, // Attempt to trigger an already triggered operation
    OE_EBUFFERSIZE      = -14, // Supplied buffer is too small
    OE_EBADDEVMAP       = -15, // Badly formated device map supplied by firmware
    OE_EBADALLOC        = -16, // Bad dynamic memory allocation
    OE_ECLOSEFAIL       = -17, // File descriptor close failure, check errno
    OE_EREADONLY        = -18, // Attempted write to read only object (register, context option, etc)
    OE_EUNIMPL          = -19, // Specified, but unimplemented, feature
    OE_EINVALREADSIZE   = -20, // Block read size is smaller than the maximal frame size
} oni_error_t;
```

## oni_create_ctx
Create a hardware context. A context is an opaque handle to a structure which
contains hardware and device state information, configuration capabilities, and
data format information. It can be modified via calls to `oni_set_opt`. Its
state can be examined by `oni_get_opt`.

``` {.c}
oni_ctx oni_create_ctx()
```

### Returns `oni_ctx`
An opaque handle to the newly created context if successful. Otherwise it shall
return NULL and set errno to `EAGAIN`.

### Description
On success a context struct is allocated and created, and its handle is passed
to the user. The context holds all state used by the library function calls for
refection and hardware communication. It holds paths to FIFOs and configuration
communication channels and knowledge of the hardware's parameters and run state
. It is configured through calls to `oni_set_opt`. It can be examined through
calls to `oni_get_opt`.

## oni_init_ctx
Initialize a context, opening all file streams etc.

``` {.c}
int oni_init_ctx(oni_ctx ctx)
```

### Arguments
- `ctx` context

### Returns `int`
- 0: success
- Less than 0: `oni_error_t`

### Description
Upon a call to `oni_init_ctx`, the following actions take place

1. All required data streams are opened.
2. A device map is read from the firmware. It can be examined via calls t
   `oni_get_opt`.
3. The data transmission packet size is calculated and stored. It can be
   examined via calls to `oni_get_opt`.

Following a successful call to `oni_init_ctx`, the hardware's acquisition
parameters and run state can be manipulated using calls to `oni_get_opt`.

## oni_destroy_ctx
Terminate a context and free bound resources.

``` {.c}
int oni_destroy_ctx(oni_ctx ctx)
```

### Arguments
- `ctx` context

### Returns `int`
- 0: success
- Less than 0: `oni_error_t`

### Description
During context destruction, all resources allocated by `oni_create_ctx` are
freed. This function can be called from any context run state. When called, an
interrupt signal (TODO: Which?) is raised and any blocking operations will
return immediately. Attached resources (e.g. file descriptors and allocated
memory) are closed and their resources freed.

## oni_get_opt
Get context options.

``` {.c}
int oni_get_opt(const oni_ctx ctx, int option, void* value, size_t *size);
```

### Arguments
- `ctx` context to read from
- `option` option to read
- `value` buffer to store value of `option`
- `size` pointer to the size of `value` (including terminating null character,
  if applicable) in bytes

### Returns `int`
- 0: success
- Less than 0: `oni_error_t`

### Description
The `oni_get_opt` function sets the option specified by the `option` argument
to the value pointed to by the `value` argument for the context pointed to by
the `ctx` argument. The `size` provides a pointer to the size of the option
value in bytes. Upon successful completion `oni_get_opt` shall modify the value
pointed to by `size` to indicate the actual size of the option value stored in
the buffer.

Following a successful call to `oni_init_ctx`, the following socket options
can be read:

#### `OE_CONFIGSTREAMPATH`\*
Obtain path specifying config data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the configuration stream path |
| default value       | /dev/xillybus_oni_config_32, \\\\.\\xillybus_oni_config_32 (Windows) |

#### `OE_READSTREAMPATH`\*
Obtain path specifying input data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the input stream path |
| default value       | /dev/xillybus_oni_input_32 \\\\.\\xillybus_oni_input_32 (Windows) |

#### `OE_WRITESTREAMPATH`\*
Obtain path specifying input data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the output stream path |
| default value       | /dev/xillybus_oni_output_32, \\\\.\\xillybus_oni_output_32 (Windows) |

#### `OE_SIGNALSTREAMPATH`\*
Obtain path specifying hardware signal data stream

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the signal stream path |
| default value       | /dev/xillybus_oni_signal_8, \\\\.\\xillybus_oni_signal_8 (Windows) |

#### `OE_DEVICEMAP`
The device map.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oni_device_t *` |
| option description  | Pointer to a pre-allocated array of `oni_device_t` structs |
| default value       | N/A |

#### `OE_NUMDEVICES`
The number of devices in the device map.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oni_reg_val_t` |
| option description  | The number of devices supported by the firmware |
| default value       | N/A |

#### `OE_MAXREADFRAMESIZE`
The maximal size of a frame produced by a call to `oni_read_frame` in bytes.
This number is the size of the frame produced by every device within the device
map that generates read data.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oni_reg_val_t` |
| option description  | Maximal read frame size in bytes |
| default value       | N/A |

#### `OE_WRITEFRAMESIZE`
The maximal size of a frame accepted by a call to `oni_write_frame` in bytes.
This number is the size of the frame provided to `oni_write_frame` to update
all output devices synchronously.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oni_reg_val_t` |
| option description  | Maximal write frame size in bytes |
| default value       | N/A |

#### `OE_RUNNING`
Hardware acquisition run state. Any value greater than 0 indicates that acquisition is
running.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oni_reg_val_t` |
| option description  | Any value greater than 0 will start acquisition |
| default value       | False |

#### `OE_SYSCLKHZ`
System clock frequency in Hz. The PCIe bus is operated at this rate. Read-frame clock values
are incremented at this rate.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oni_reg_val_t` |
| option description  | System clock frequency in Hz |
| default value       | N/A |

#### `OE_ACQCLKHZ`
Acquisition clock frequency in Hz. Reads from devices are synchronized to this clock.
Clock values within frame data are incremented at this rate.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oni_reg_val_t` |
| option description  | Acquisition clock frequency in Hz |
| default value       | 42000000 |


#### `OE_BLOCKREADSIZE`
Number of bytes read during each `read()` syscall to the data read stream. This
option allows control over a fundamental trade-off between closed-loop response
time and overall performance. The minimum (default) value will provide the
lowest response latency. Larger values will reduce syscall frequency and may
improve processing performance for high-bandwidth data sources.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `size_t` |
| option description  | Size, in bytes, of `read()` syscalls |
| default value       | value of `OE_MAXREADFRAMESIZE` |

## oni_set_opt
Set context options.

``` {.c}
int oni_set_opt(oni_ctx ctx, int option, const void* value, size_t size);
```

### Arguments
- `ctx` context
- `option` option to set
- `value` value to set `option` to
- `size` length of `value` in bytes

### Returns `int`
- 0: success
- Less than 0: `oni_error_t`

### Description
The `oni_set_opt` function sets the option specified by the `option` argument to
the value pointed to by the `value` argument within `ctx`. The `size` indicates
the size of the `value` in bytes.

The following context options can be set:

#### `OE_CONFIGSTREAMPATH`\*
Set path specifying configuration data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the configuration stream path |
| default value       | /dev/xillybus_oni_config_32, \\\\.\\xillybus_oni_config_32 (Windows) |

#### `OE_READSTREAMPATH`\*
Set path specifying input data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the input stream path |
| default value       | /dev/xillybus_oni_input_32, \\\\.\\xillybus_oni_input_32 (Windows) |

#### `OE_WRITESTREAMPATH`\*
Set path specifying input data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the output stream path |
| default value       | /dev/xillybus_oni_output_32, \\\\.\\xillybus_oni_output_32 (Windows) |

#### `OE_SIGNALSTREAMPATH`\*
Set path specifying hardware signal data stream

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the signal stream path |
| default value       | /dev/xillybus_oni_signal_8, \\\\.\\xillybus_oni_signal_8 (Windows) |

#### `OE_RUNNING`\*\*
Set/clear data input gate. Any value greater than 0 will start acquisition.
Writing 0 to this option will stop acquisition, but will not reset context
options or the sample counter. All data not shifted out of hardware will be
cleared. To obtain the very first samples produced by high-bandwidth devices,
set `OE_RUNNING` _before_ a call to OE_RESET.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type         | `oni_reg_val_t` |
| option description        | Any value greater than 0 will start acquisition |
| default value             | 0 |

#### `OE_RESET`\*\*
Trigger global hardware reset. Any value great than 0 will trigger a hardware
reset. In this case, acquisition is stopped and all global hardware state (e.g.
sample counters, etc) is defaulted.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type         | `oni_reg_val_t` |
| option description        | Any value greater than 0 will trigger a reset |
| default value             | Untriggered |

#### `OE_BLOCKREADSIZE`\*\*\*
Number of bytes read during each `read()` syscall to the data read stream. This
option allows control over a fundamental trade-off between closed-loop response
time and overall performance. The minimum (default) value will provide the
lowest response latency. Larger values will reduce syscall frequency and may
improve processing performance for high-bandwidth data sources.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `size_t` |
| option description  | Size, in bytes, of `read()` syscalls |
| default value       | value of `OE_MAXREADFRAMESIZE` |

\* Invalid following a successful call to `oni_init_ctx`. Before this, will
return with error code `OE_EINVALSTATE`.

\*\* Invalid until a successful call to `oni_init_ctx`. After this, will
return with error code `OE_EINVALSTATE`.

\*\*\* Invalid until a successful call to `oni_init_ctx` and before acquisition
is started by setting the `OE_RUNNING` context option. In other states, will
return with error code `OE_EINVALSTATE`.

## oni_read_reg
Read a configuration register on a specific device.

``` {.c}
int oni_read_reg(const oni_ctx ctx, size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t *value);
```

### Arguments
- `ctx` context
- `dev_idx` physical index number
- `addr` The address of register to write to
- `value` pointer to an int that will store the value of the register at `addr`
  on `dev_idx`. This contents of this pointer will first be written to register
  programming bus, since some devices need to use it to recieve a valid read.
  e.g. using an SPI bus where reads are initialized by the value on MOSI one
  transation prior.

### Returns `int`
- 0: success
- Less than 0: `oni_error_t`

### Description
`oni_read_reg` is used to read the value of configuration registers from devices
within the current device map. This can be used to verify the success of calls
to `oni_read_reg` or to obtain state information about devices managed by the
current context.

## oni_write_reg
Set a configuration register on a specific device.

``` {.c}
int oni_write_reg(const oni_ctx ctx, size_t dev_idx, oni_reg_addr_t addr, oni_reg_val_t value);
```

### Arguments
- `ctx` context
- `dev_idx` the device index to read from
- `addr` register address within the device specified by `dev_idx` to write
  to
- `value` value with which to set the register at `addr` on the device
  specified by `dev_idx`

### Returns `int`
- 0: success
- Less than 0: `oni_error_t`

### Description
`oni_write_reg` is used to write the value of configuration registers from
devices within the current device map. This can be used to set configuraiton
registers for devices managed by the current context. For example, this is used
to perform configuration of ADCs that exist in a device map. Note that
successful return from this function does not guarantee that the register has
been properly set. Confirmation of the register value can be made using a call
to `oni_read_reg`.

## oni_read_frame
Read high-bandwidth data from the read channel.

``` {.c}
int oni_read_frame(const oni_ctx ctx, oni_frame_t **frame)
```

### Arguments
- `ctx` context
- `frame` Pointer to a `oni_frame_t` pointer

### Returns `int`
- 0: success
- Less than 0: `oni_error_t`

### Description
`oni_read_frame` allocates host memory and populates it with an `oni_frame_t`
struct corresponding to a single [frame](#frame), with a read header, from the
data input channel. This call will block until either enough data to construct
a frame is available on the data input stream or
[`oni_destroy_ctx`](#oni_destroy_ctx) is called. It is the user's repsonisbility
to free the resources allocated by this call by passing the resulting frame
pointer to [`oni_destroy_frame`](#oni_destroy_frame).

## oni_destroy_frame
Free heap-allocated frame.

```{.c}
void oni_destroy_frame(oni_frame_t *frame);
```

### Arguments
- `frame` pointer to an `oni_frame_t`

### Returns `void`
There is no return value.

### Description
`oni_destroy_frame` frees a heap-allocated frame. It is generally used to clean
up the resources allocated by [`oni_read_frame`](#oni_read_frame).

## oni_write
Write data to the data write channel.

``` {.c}
int oni_write_frame(const oni_ctx ctx, size_t dev_idx, const void *data, size_t data_sz)
```

### Arguments
- `ctx` context
- `dev_idx` device to write to
- `data` pointer to data to write
- `data_sz` number of bytes to write

### Returns `int`
- 0: success
- Less than 0: `oni_error_t`

### Description
`oni_write_frame` writes block data to a particular device from the device map
using the asynchronous data write channel. If `dev_idx` is not a writable
device, or if `data_sz` does not equal to `write_size` the of the device, this
function will return `OE_EDEVIDX` and `OE_EWRITESIZE`, respectively.

## oni_version
Report the oepcie library version.

``` {.c}
void oni_version(int major, int minor, int patch)
```

### Arguments
- `major` major library version
- `minor` minor library version
- `patch` patch number

### Returns `void`
There is no return value.

### Description
This library uses [semantic versioning](www.semver.org). Briefly, the major
revision is for incompatible API changes. Minor version is for backwards
compatible changes. The patch number is for backwards-compatible bug fixes.

## oni_error_st
Convert an [error number](#oni_error_t) into a human readable string.

``` {.c}
const char *oni_error_str(int err)
```

### arguments
- `err` error code

### returns `const char *`
Pointer to an error message string

## oni_device_str
Convert a [device ID](#oni_device_t) into human readable string. _Note_: This is
an extension function available in oedevices.h.

``` {.c}
const char *oni_device(ind dev_id)
```

### Arguments
- `dev_id` device id

### Returns `const char *`
Pointer to a device id string
