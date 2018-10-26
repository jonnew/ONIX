# Open Ephys++ API

1. The [Open-ephys++ API specification](spec.pdf) which defines the
   requirements of a host API that communicates with hardware in this project

2. API implementations based upon this specification:

    - [liboepcie](liboepcie) is an ANSI-C open-ephys++ API implementation.
    It contains functions for configuring and stream data to and from hardware.
    - [cppoepcie](cppoepcie) C++14 bindings for liboepcie.
    - [clroepcie](clroepcie) CLR/.NET bindings for liboepcie.

    Example host programs for each of these libraries can be found in the
    \*-test folder within each library directory.

## License
[MIT](https://en.wikipedia.org/wiki/MIT_License)

## Using Xillybus with `liboepcie`
Instructions for using [Xillybus](http://xillybus.com/) as a communication
backend can be found [here](xillybus-backend.md).

## `liboepcie`: Detailed Description

`liboepcie` is a C library that implements the [Open Ephys++ API
Specification](spec.pdf). It is written in ANSI-C to facilitate
cross platform and cross-language use. It is composed of two mutually
exclusive file pairs:

1. oepcie.h and oepcie.c: main API implementation
1. oedevice.h and oedevices.c: officially supported device and register
   definitions. This file can be ignored for project that do not wish to
   conform to the official device specification.

`liboepcie` is a low level library used by high-level language binding and/or
software plugin developers. It is not meant to be used by neuroscientists
directly. The only external dependency aside from the C standard library is is
a hardware communication backend that fulfills the requirements of the
FPGA/Host Communication Specification. An example of such a backend is
[Xillybus](http://xillybus.com/), which provides proprietary FPGA IP cores and
free and open source device drivers to allow the communication channels to be
implemented using the PCIe bus. From the API's perspective, hardware
communication abstracted to IO system calls (`open`, `read`, `write`, etc.) on
file descriptors. File descriptor semantics and behavior are identical to
either normal files (configuration channel) or named pipes (signal, data input,
and data output channels). Because of this, a drop in replacement for the
Xillybus IP Core can be used without any API changes.  The development of a
free and open-source FPGA cores that emulate the functionality of Xillybus
would be a major benefit to the systems neuroscience community.

Importantly, the low-level synchronization, resource allocation, and logic
required to use the hardware communication backend is implicit to `liboepcie`
API function calls. Orchestration of the communication backend _is not directly
managed by the library user_.

## Types

### Integer types
- `oe_size_t`: Fixed width size integer type.
- `oe_dev_id_t`: Fixed width device identity integer type.
- `oe_reg_addr_t`: Fixed width device register address integer type.
- `oe_reg_value_t`: Fixed width device register value integer type.

### `oe_ctx`
[Context](#context) implementation. `oe_ctx` is an opaque handle to a context
structure which contains hardware and device state information.

```
// oepcie.h
typedef struct oe_ctx_impl *oe_ctx;
```

Context details are hidden in implementation file (oepcie.c):

``` {.c}
typedef struct stream_fid {
    char *path;
    int fid;
} stream_fid_t;

typedef struct oe_ctx_impl {

    // Communication channels
    stream_fid_t config;
    stream_fid_t read;
    stream_fid_t write;
    stream_fid_t signal;

    // Devices
    oe_size_t num_dev;
    oe_device_t* dev_map;

    // Maximum frame sizes (bytes)
    oe_size_t max_read_frame_size;
    oe_size_t write_frame_size;

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

} oe_ctx_impl_t;
```

Each context manages a single device map. Following a hardware reset, which is
triggered either by a call to `oe_init_ctx` or to `oe_set_ctx` using the
`OE_RESET` option, the context `run_state` is set to UNINTIALIZED and the device map is
pushed onto the signal stream by the FPGA as
[COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) encode
packets. On the signal stream, the device map is organized as follows,

```
... | DEVICEMAPACK, uint32_t num_devices | DEVICEINST oe_device_t dev_0
    | DEVICEINST oe_device_t dev_1 | ... | DEVICEINST oe_device_t dev_n | ...
```

where | represents '0' packet delimiters. During a call to `oe_init_ctx`, the
device map is decoded from the signal stream. It can then be examined using
calls to `oe_get_opt` using the `OE_DEVICEMAP` option. After the map is
received, the context `run_state` becomes IDLE. A call to `oe_set_ctx` with the
`OE_RUNNING` option can then be used to start acquisition by transitioning the
context `run_state` to `RUNNING`.

### `oe_device_t`
[Device](#device) implementation. An `oe_device_t` describes one of potentially
many pieces of hardware within a context. Examples include Intan chips, IMUs,
optical stimulator's, camera sensors, etc. Each valid device type has a unique
ID which is enumerated in the auxiliary `oedevices.h` file or some use-specific
header. A map of available devices is read from hardware and stored in the
current context via a call to [`oe_init_ctx`](#oe_init_ctx).  This map can be
examined via calls to [`oe_get_opt`](#oe_get_opt).

``` {.c}
typedef struct {
    oe_dev_id_t id;         // Device ID number
    oe_size_t read_size;    // Device data read size per frame in bytes
    oe_size_t num_reads;    // Number of read frames to construct a full sample
    oe_size_t write_size;   // Device data write size per frame in bytes
    oe_size_t num_writes;   // Number of written frames comprising a full sample
} oe_device_t;
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
} oe_device_id_t

```

An example of a device register (for the `OE_ESTIM` device ID) enumeration is:

``` {.c}
enum oe_estim_regs {
    OE_ESTIM_NULLPARM    = 0,  // No command
    OE_ESTIM_BIPHASIC    = 1,  // Biphasic pulse (0 = monophasic, 1 = biphasic;
    OE_ESTIM_CURRENT1    = 2,  // Phase 1 current, (0 to 255 = -1.5 mA to +1.5mA)
    OE_ESTIM_CURRENT2    = 3,  // Phase 2 voltage, (0 to 255 = -1.5 mA to +1.5mA)
    OE_ESTIM_PULSEDUR1   = 4,  // Phase 1 duration, 10 microsecond steps
    ...
};
```

These registers may be familiar to those who have used a Master-8 or
[pulse-pal](https://sites.google.com/site/pulsepalwiki/) stimulus sequencer.

### `oe_frame_t`
[Frame](#frame) implementation. Frames are produced by calls `oe_read_frame`
and provided to calls to `oe_write_frame`.

``` {.c}
typedef struct oe_frame {
    uint64_t clock;         // Base clock counter
    uint16_t num_dev;       // Number of devices in frame
    uint8_t corrupt;        // Is this frame corrupt?
    oe_size_t dev_idxs[];   // Fixed-size array of device indices in frame
    oe_size_t dev_offs[];   // Fixed-size data offsets within data block
    uint8_t *data;          // Multi-device raw data block
    oe_size_t data_sz;      // Size in bytes of data buffer

} oe_frame_t;
```

### `oe_opt_t`
Context option enumeration. See the description of `oe_set_opt` and
`oe_get_opt` for valid values.

### `oe_error_t`
Error code enumeration.

``` {.c}
typedef enum oe_error {
    OE_ESUCCESS         =  0,  // Success
    OE_EPATHINVALID     = -1,  // Invalid stream path, fail on open
    OE_EREINITCTX       = -2,  // Double initialization attempt
    ...
} oe_error_t;
```

## oe_create_ctx
Create a hardware context. A context is an opaque handle to a structure which
contains hardware and device state information, configuration capabilities, and
data format information. It can be modified via calls to `oe_set_opt`. Its
state can be examined by `oe_get_opt`.

``` {.c}
oe_ctx oe_create_ctx()
```

### Returns `oe_ctx`
An opaque handle to the newly created context if successful. Otherwise it shall
return NULL and set errno to `EAGAIN`.

### Description
On success a context struct is allocated and created, and its handle is passed
to the user. The context holds all state used by the library function calls for
refection and hardware communication. It holds paths to FIFOs and configuration
communication channels and knowledge of the hardware's parameters and run state
. It is configured through calls to `oe_set_opt`. It can be examined through
calls to `oe_get_opt`.

## oe_init_ctx
Initialize a context, opening all file streams etc.

``` {.c}
int oe_init_ctx(oe_ctx ctx)
```

### Arguments
- `ctx` context

### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### Description
Upon a call to `oe_init_ctx`, the following actions take place

1. All required data streams are opened.
2. A device map is read from the firmware. It can be examined via calls t
   `oe_get_opt`.
3. The data transmission packet size is calculated and stored. It can be
   examined via calls t `oe_get_opt`.

Following a successful call to `oe_init_ctx`, the hardware's acquisition
parameters and run state can be manipulated using calls to `oe_get_opt`.

## oe_destroy_ctx
Terminate a context and free bound resources.

``` {.c}
int oe_destroy_ctx(oe_ctx ctx)
```

### Arguments
- `ctx` context

### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### Description
During context destruction, all resources allocated by `oe_create_ctx` are
freed. This function can be called from any context run state. When called, an
interrupt signal (TODO: Which?) is raised and any blocking operations will
return immediately. Attached resources (e.g. file descriptors and allocated
memory) are closed and their resources freed.

## oe_get_opt
Get context options.

``` {.c}
int oe_get_opt(const oe_ctx ctx, int option, void* value, size_t *size);
```

### Arguments
- `ctx` context to read from
- `option` option to read
- `value` buffer to store value of `option`
- `size` pointer to the size of `value` (including terminating null character,
  if applicable) in bytes

### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### Description
The `oe_get_opt` function sets the option specified by the `option` argument
to the value pointed to by the `value` argument for the context pointed to by
the `ctx` argument. The `size` provides a pointer to the size of the option
value in bytes. Upon successful completion `oe_get_opt` shall modify the value
pointed to by `size` to indicate the actual size of the option value stored in
the buffer.

Following a successful call to `oe_init_ctx`, the following socket options
can be read:

#### `OE_CONFIGSTREAMPATH`\*
Obtain path specifying config data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the configuration stream path |
| default value       | /dev/xillybus_oe_config_32, \\\\.\\xillybus_oe_config_32 (Windows) |

#### `OE_READSTREAMPATH`\*
Obtain path specifying input data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the input stream path |
| default value       | /dev/xillybus_oe_input_32 \\\\.\\xillybus_oe_input_32 (Windows) |

#### `OE_WRITESTREAMPATH`\*
Obtain path specifying input data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the output stream path |
| default value       | /dev/xillybus_oe_output_32, \\\\.\\xillybus_oe_output_32 (Windows) |

#### `OE_SIGNALSTREAMPATH`\*
Obtain path specifying hardware signal data stream

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the signal stream path |
| default value       | /dev/xillybus_oe_signal_8, \\\\.\\xillybus_oe_signal_8 (Windows) |

#### `OE_DEVICEMAP`
The device map.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type	  | `oe_device_t *` |
| option description  | Pointer to a pre-allocated array of `oe_device_t` structs |
| default value	      | N/A |

#### `OE_NUMDEVICES`
The number of devices in the device map.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oe_reg_val_t` |
| option description  | The number of devices supported by the firmware |
| default value       | N/A |

#### `OE_MAXREADFRAMESIZE`
The maximal size of a frame produced by a call to `oe_read_frame` in bytes.
This number is the size of the frame produced by every device within the device
map that generates read data.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oe_reg_val_t` |
| option description  | Maximal read frame size in bytes |
| default value       | N/A |

#### `OE_WRITEFRAMESIZE`
The maximal size of a frame accepted by a call to `oe_write_frame` in bytes.
This number is the size of the frame provided to `oe_write_frame` to update
all output devices synchronously.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oe_reg_val_t` |
| option description  | Maximal write frame size in bytes |
| default value       | N/A |

#### `OE_RUNNING`
Hardware acquisition run state. Any value greater than 0 indicates that acquisition is
running.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oe_reg_val_t` |
| option description  | Any value greater than 0 will start acquisition |
| default value       | False |

#### `OE_SYSCLKHZ`
System clock frequency in Hz. The PCIe bus is operated at this rate. Read-frame clock values
are incremented at this rate.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oe_reg_val_t` |
| option description  | System clock frequency in Hz |
| default value       | N/A |

#### `OE_ACQCLKHZ`
Acquisition clock frequency in Hz. Reads from devices are synchronized to this clock.
Clock values within frame data are incremented at this rate.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `oe_reg_val_t` |
| option description  | Acquisition clock frequency in Hz |
| default value       | 42000000 |


## oe_set_opt
Set context options.

``` {.c}
int oe_set_opt(oe_ctx ctx, int option, const void* value, size_t size);
```

### Arguments
- `ctx` context
- `option` option to set
- `value` value to set `option` to
- `size` length of `value` in bytes

### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### Description
The `oe_set_opt` function sets the option specified by the `option` argument to
the value pointed to by the `value` argument within `ctx`. The `size` indicates
the size of the `value` in bytes.

The following context options can be set:

#### `OE_CONFIGSTREAMPATH`\*
Set path specifying configuration data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the configuration stream path |
| default value       | /dev/xillybus_oe_config_32, \\\\.\\xillybus_oe_config_32 (Windows) |

#### `OE_READSTREAMPATH`\*
Set path specifying input data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the input stream path |
| default value       | /dev/xillybus_oe_input_32, \\\\.\\xillybus_oe_input_32 (Windows) |

#### `OE_WRITESTREAMPATH`\*
Set path specifying input data stream.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the output stream path |
| default value       | /dev/xillybus_oe_output_32, \\\\.\\xillybus_oe_output_32 (Windows) |

#### `OE_SIGNALSTREAMPATH`\*
Set path specifying hardware signal data stream

| | |
|---------------------|--------------------------------------------------------------------|
| option value type   | `char *` |
| option description  | A character string specifying the signal stream path |
| default value       | /dev/xillybus_oe_signal_8, \\\\.\\xillybus_oe_signal_8 (Windows) |

#### `OE_RUNNING`\*\*
Set/clear master clock gate. Any value greater than 0 will start acquisition.
Writing 0 to this option will stop acquisition, but will not reset context
options or the sample counter.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type 	    | `oe_reg_val_t` |
| option description 	    | Any value greater than 0 will start acquisition |
| default value     	    | 0 |

#### `OE_RESET`\*\*
Trigger global hardware reset. Any value great than 0 will trigger a hardware
reset. In this case, acquisition is stopped and all global hardware state (e.g.
sample counters, etc) is defaulted.

| | |
|---------------------|--------------------------------------------------------------------|
| option value type 	    | `oe_reg_val_t` |
| option description 	    | Any value greater than 0 will trigger a reset |
| default value     	    | Untriggered |


\* Invalid following a successful call to `oe_init_ctx`. Before this, will
return with error code `OE_EINVALSTATE`.

\*\* Invalid until a successful call to `oe_init_ctx`. After this, will
return with error code `OE_EINVALSTATE`.

## oe_read_reg
Read a configuration register on a specific device.

``` {.c}
int oe_read_reg(const oe_ctx ctx, size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t *value);
```

### Arguments
- `ctx` context
- `dev_idx` physical index number
- `addr` The address of register to write to
- `value` pointer to an int that will store the value of the register at `addr` on `dev_idx`

### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### Description
`oe_read_reg` is used to read the value of configuration registers from devices
within the current device map. This can be used to verify the success of calls
to `oe_read_reg` or to obtain state information about devices managed by the
current context.

## oe_write_reg
Set a configuration register on a specific device.

``` {.c}
int oe_write_reg(const oe_ctx ctx, size_t dev_idx, oe_reg_addr_t addr, oe_reg_val_t value);
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
- Less than 0: `oe_error_t`

### Description
`oe_write_reg` is used to write the value of configuration registers from
devices within the current device map. This can be used to set configuraiton
registers for devices managed by the current context. For example, this is used
to perform configuration of ADCs that exist in a device map. Note that
successful return from this function does not guarantee that the register has
been properly set. Confirmation of the register value can be made using a call
to `oe_read_reg`.

## oe_read_frame
Read high-bandwidth input data stream.

``` {.c}
int oe_read_frame(const oe_ctx ctx, oe_frame_t **frame)
```

### Arguments
- `ctx` context
- `frame` Pointer to a `oe_frame_t` pointer

### Returns `int`
- Greater than 0: Success. Return code indicates total frame size in bytes.
- Less than 0: `oe_error_t`

### Description
`oe_read_frame` allocates host memory and populates it with an `oe_frame_t`
struct corresponding to a single [frame](#frame), with a read header, from the
data input channel. This call will block until either enough data to construct
a frame is available on the data input stream or
[`oe_destroy_ctx`](#oe_destroy_ctx) is called. It is the user's repsonisbility
to free the resources allocated by this call by passing the resulting frame
pointer to [`oe_destroy_frame`](#oe_destroy_frame).

## oe_write_frame
Write a frame to the output data channel.

``` {.c}
int oe_write_frame(const oe_ctx ctx, oe_frame_t *frame)
```

### Arguments
- `ctx` context
- `frame` pointer to an `oe_frame_t`

### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### Description
`oe_write_frame` writes a pre-allocated and populated `stuct` corresponding to
a single [frame](#frame), with a write header, into the asynchronous data
output channel from host memory. If the frame specifies that devices without
write capabilities should be written to, this function will return
`OE_EWRITEFAILURE`.

## oe_destroy_frame
Free heap-allocated frame.

```{.c}
void oe_destroy_frame(oe_frame_t *frame);
```

### Arguments
- `frame` pointer to an `oe_frame_t`

### Returns `void`
There is no return value.

### Description
`oe_destroy_frame` frees a heap-allocated frame. It is generally used to clean
up the resources allocated by [`oe_read_frame`](#oe_read_frame) or allocated by
the caller for [`oe_write_frame`](#oe_write_frame).

## oe_version
Report the oepcie library version.

``` {.c}
void oe_version(int major, int minor, int patch)
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

## oe_error_st
Convert an [error number](#oe_error_t) into a human readable string.

``` {.c}
const char *oe_error_str(int err)
```

### arguments
- `err` error code

### returns `const char *`
Pointer to an error message string

## oe_device_str
Convert a [device ID](#oe_device_t) into human readable string. _Note_: This is
an extension function available in oedevices.h.

``` {.c}
const char *oe_device_str(ind dev_id)
```

### Arguments
- `dev_id` device id

### Returns `const char *`
Pointer to a device id string
