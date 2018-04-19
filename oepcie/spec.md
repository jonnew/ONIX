---
title: |
    Open Ephys++ Communication Protocol and API Specification Version 1.0
author:
    - Jonathan P. Newman*
date: \today{}
geometry: margin=2cm
header-includes:
    - \usepackage{setspace}
    - \usepackage{lineno}
    - \linenumbers
abstract: |
    Configure hardware and read and write streaming data from and to a host
    applications.
---

# API Specification

## License
[MIT](https://en.wikipedia.org/wiki/MIT_License)

## Design Requirements
- Low latency (sub millisecond)
- High bandwidth (> 1000 neural data channels)
- Support for different hardware components on a single PCIe bus
    - Support generic mixes of hardware elements
    - Generic configuration channels
    - Generic data input stream
    - Generic data output stream
- Usable by different host applications
- Support multiple PCIe devices on one PC
- Cross platform

## Language
The implementation is in C to facilitate cross platform and cross-language use.

## Scope and External Dependencies
- This is a low level library used by high-level language binding and/or
  application plugin developers. It is not meant to be used by neuroscientists
  directly.

- The only external dependency aside from the c standard library is
  [Xillybus](http://xillybus.com/), which will be used for PCIe communication.
  Because xillybus provides abstracts this functionality to the level of file
  IO, a drop in replacement without its commercial restrictions is possible if
  open-ephys ever gathers the funds to do so.

- The public programming interface, supported device ID enumeration, and public
  type declarations should be limited to a single header file.

## Types and Definitions

### Context
A _context_ (`oe_ctx`) holds all required state for a devices associated with
one PCIe bus in the following hierarchy:

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

State details are hidden in implementation file. A pointer to opaque type
(handle) is exposed publicly in the header (oepcie.h):

```
// oepcie.h
typedef struct oe_ctx_impl *oe_ctx;
```

API calls will typically take a context handle as the first argument and use it
to reference required state information to enable communication and/or to
mutate the context to reflect some function side effect (e.g. add device map
information):

```
int oe_api_function(oe_ctx ctx, ...);
```

### Device 
A _device_ (`oe_device_t`) is defined as configurable piece of
hardware with its own register address space (e.g. an integrated circuit) or
something programmed within the firmware to emulate this (e.g. an electrical
stimulation sub-circuit made to look like a Master-8). Host interaction with a
device is facilitated using a device description, which is provided by a
`struct` as follows:

``` {.c}
typedef struct {
    oe_dev_id_t id;         // Device ID number (see oedevices.h)
    oe_size_t read_size;    // Device data read size per frame in bytes
    oe_size_t num_reads;    // Number of frames that must be read to construct a full sample (e.g., for row reads from camera)
    oe_size_t write_size;   // Device data write size per frame in bytes
    oe_size_t num_writes;   // Number of frames that must be written to construct a full output sample

} oe_device_t;
```

The definition of each member of the `oe_device_t` structure is provided below:

1. `enum config_device_id`: Device identification number which is globally
   enumerated for the entire project
    - e.g. Immediate IO (`OE_IMMEDIATEIO`) from the host board is 0
    - e.g. Intan RHD2032 (`OE_RHD2132`) is 1, Intan RHD2064 (`OE_RHUD21641`) is
      2, etc.
    - Device IDs are defined in `oedevices.h`
    - This enumeration grows with the number of devices supported by the
      library. There is a single `enum` for the entire library which
      enumerates all possible devices that are controlled across `context`
      configurations.
    - Device IDs up to 9999 are reserved. Device ID 10000 and greater are free
      to use for custom projects
    - A `context` is only responsible for controlling a subset of all
      supported devices. This subset is referred to a _device map_.

    ```
    typedef enum device_id {
        OE_IMMEDIATEIO = 0,
        OE_RHD2132,
        OE_RHD2164,
        OE_MPU9250,
        ...
        OE_MAXDEVICEID       = 9999
    } oe_device_id_t

    ```
    - The use of device IDs less than 10000 not specified within this
      enumeration will result in OE_EDEVID errors.
    - Device numbers greater than 1000 are allowed for general purpose use and
      will not be verified by the API.
    - Incorporation into the official device enum (device IDs < 10000) can be
      achieved via pull-request to this repo.

2. `oe_size_t read_size`: bytes of each transmitted data frame from this device.
    - 0 indicates that it does not send data.

3. `oe_size_t num_reads`: Number of frames that must be read to construct a full sample (e.g., for row reads from camera.

4. `oe_size_t write_size`: bytes within the output frame transmitted data packet to this device.
    - 0 indicates that it does not send data.

5. `oe_size_t num_writes`: number of frames that must be written to construct a full output sample.

Following a hardware reset, which is triggered either by a call to
`oe_init_ctx` or to `oe_set_ctx` using the `` option, the device map is pushed
onto the signal stream by the FPGA as
[COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) encoded
packets. On the signal stream, the device map is organized as follows,

... | DEVICEMAPACK, uint32_t num_devices | DEVICEINST oe_device_t dev_0
    | DEVICEINST oe_device_t dev_1 | ... | DEVICEINST oe_device_t dev_n | ...

where | represents '0' packet delimiters. During a call to `oe_init_ctx`, the
device map is decoded from the signal stream. It can then be examined using
calls to `oe_get_opt` using the `OE_DEVICEMAP` option.

### Frame
A _frame_ is a flat byte array containing a single sample's worth of data for a
set (one to all) of devices within a device map. Frames are produced by calls
`oe_read_frame()` and provided to calls to `oe_write_frame()`. Data within
frames is arranged into three sectors as follows:

```
[32 byte header,                              // 1. header
 dev_0 idx, dev_1 idx, ... , dev_n idx,       // 2. device map indicies
 dev_0 data, dev_1 data, ... , dev_n data]    // 3. data

```

Each frame memory sector is described below:

1. header
    - Each frame starts with a 32-byte header
    - For reading ([`oe_read_frame()`](#oe-read-frame)), the header contains
        - bytes 0-7: `uint64_t` system clock counter
        - bytes 8-9: `uin16_t` indicating number of devices that the frame contains
          data for
        - byte 10: `int8_t` frame error. 0 = OK. 1 = data may be corrupt.
        - bytes 11-32: reserved

    - For writing ([`oe_write_frame()`](#oe-write-frame)), the header contains
        - bytes 0-32: reserved

2. device map indices
    - An array of `uint32_t` indicies into the device map captured by the host
      during a call to `oe_init_ctx()`.
    - The offset, size, and type information of the _i_th data block within the
      `data` section of each frame is determined by examining the _i_th member
      of the device map.

3. data
    - Raw data blocks from each device in the device map.
    - The ordering of device-specific blocks is the same as the device index
      within the _device map index_ portion of the frame
    - The read/write size for each device-specific block is provided in the
      device map
    - Data block type information is provided in device specific manual file,
      in this repository (TODO)
    - Perhaps in the future, data type casting information can be provided in
      the device map, but this is not currently implemented

# FPGA Board/Host PC Communication Protocol
FPGA/host communication occurs over four distinct channels. Communication over
these channels is implicit to API calls and is _not directly managed by the
programmer_. Under the hood, the library interacts with each channels using
standard UNIX-like file system calls (`open()`, `close()_`, `read()`,
`write()`, etc.). Their semantics and behavior are identical to either normal
files (configuration channel) or named pipes (signal, data input, and data
output channels).

## Signal channel (8-bit, asynchronous, read only)
The _signal_ channel provides a way for the FPGA firmware to inform host of
configuration results, which may be provided with a signifcant delay.
Additionally, it allows the host to read the device map supported by the FPGA
firmware. The behavior of the signal channel is equivalent to a read-only,
blocking UNIX named pipe. Signal data is framed into packets using Consistent
Overhead Byte Stuffing
([COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing)).
Within this scheme, packets are delimited using 0's and always have the
following format:

```
[PACKET-FLAG, data...]
```

where PACKET-FLAG is 32-bit unsigned integer with a single unique bit settting.
Valid PACKET-FLAGS are:

``` {.c}
typedef enum oe_signal {
    NULLSIG             = (1u << 0),
    CONFIGWACK          = (1u << 1), // Configuration write-acknowledgement
    CONFIGWNACK         = (1u << 2), // Configuration no-write-acknowledgement
    CONFIGRACK          = (1u << 3), // Configuration read-acknowledgement
    CONFIGRNACK         = (1u << 4), // Configuration no-read-acknowledgement
    DEVICEMAPACK        = (1u << 5), // Device map start acnknowledgement
    DEVICEINST          = (1u << 6), // Device map instance
} oe_signal_t;
```

Following a hardware reset (see [oe_set_opt()](#oe-set-opt)) the signal channel
is used to provide the device map and frame information to the host using the
following packet sequence:

```
... | DEVICEMAPACK, uint32_t num_devices | DEVICEINST oe_device_t dev_0
    | DEVICEINST oe_device_t dev_1 | ... | DEVICEINST oe_device_t dev_n| ...
```

Following a device register read or write (see [oe_read_reg()](#oe-read-reg)
and [oe_write_reg()](#oe-write-reg)), ACK or NACK signals are pushed onto the
signal stream by the firmware. e.g. on a successful register read:

```
... | CONFIGRACK, uint32_t register value | ...
```

## Configuration channel (32-bit, synchronous, read and write)
The _configuration_ channel supports seeking to, reading, and writing a set of
configuration registers. Its behavior is equivalent to that of a normal UNIX
file. There are two types of registers handled by the configuration channel:
the first set of registers encapsulates a generic device register programming
interface.  The remaining registers are for global context control and
configuration and provide access to acquisition parameters and state control.

1. Device register programming interface:

- `uint32_t config_device_id`: Device ID register. Specify a device endpoint as
  enumerated by the firmware (e.g. an Intan chip, or a IMU chip) and to which
  communication will be directed using `config_reg_addr` and
  `config_reg_value`, as described below.

- `uint32_t config_reg_addr`: The register address of configuration to be written

- `uint32_t config_reg_value`: configuration value to be written to or read
  from and that corresponds to `config_reg_addr` on device `config_device_id`

- `uint32_t config_rw`: A flag indicating if a read or write should be performed.
  0 indicates read operation. A value > 0 indicates write operation.

- `uint32_t config_trig`: Set > 0 to trigger either register read or write
  operation depending on the state of `config_rw`. If `config_rw` is 0, a read
  is performed. In this case `config_reg_value` is updated with value stored at
  `config_reg_addr` on device at `config_device_id`. If `config_rw` is 1,
  `config_reg_value` is written to register at `config_reg_addr` on device
  `config_device_id`. The `config_trig` register is always be set low by the
  firmware following transmission even if it is not successful or does not make
  sense given the address register values.

2. Global acquisition registers

- `uint32_t running`: set to > 0 to run the system clock and produce data. Set
  to 0 to stop the system clock and therefore stop data flow. Results in no
  other configuration changes.

- `uint32_t reset`: set to > 0 to trigger a hardware reset and send a fresh
  device map to the host and reset hardware to its default state. Set to 0 by
  host firmware upon entering the reset state.

- `uint32_t sys_clock_hz`: A read-only register specifying the base hardware
  clock frequency in Hz.

- _Note:_ Appropriate values of `config_reg_addr` and `config_reg_value` are
  determined by:
    - Looking at a device's data sheet if the device is an integrated circuit
    - Examining `oe_devices.h` header file which contains off register
      addresses and descriptions for devices officially supported by this
      project (device id < 10000).

- _Note:_ `SEEK` locations of each configuration register, relative to the
  start of the stream, are provided by the `oe_config_reg` enum.

- _Note:_ Upon a call to `oe_write_reg`, the following actions take place:

    1. The value of `config_trig` is checked.
        - If it is 0x00, the function call proceeds.
        - Else, the function call returns with OE_ERETRIG.
    1. `dev_idx` is copied to the `config_device_id` register on the host FPGA.
    1. `addr` is copied to the `config_reg_addr` register on the host FPGA.
    1. `value` is copied to the `config_reg_value` register on the host FPGA.
    1. The `config_rw` register on the host FPGA is set to 0x01.
    1. The `config_trig` register on the host FPGA is set to 0x01, triggering
    configuration transmission by the firmware.
    1. (Firmware) A configuration write is performed by the firmware.
    1. (Firmware) `config_trig` is set to 0x00 by the firmware.
    1. (Firmware) `OE_CONFIGWACK` is pushed onto the signal stream by the
    firmware.
    1. The signal stream is pumped until `OE_CONFIGWACK` is received indicating
    that the host FPGA has attempted to write to the specified device register.

- _Note:_ Upon a call to `oe_write_reg`, the following actions take place:

    1. The value of `config_trig` is checked.
        - If it is 0x00, the function call proceeds.
        - Else, the function call returns with OE_ERETRIG.
    1. `dev_idx` is copied to the `config_device_id` register on the host FPGA.
    1. `addr` is copied to the `config_reg_addr` register on the host FPGA.
    1. The `config_rw` register on the host FPGA is set to 0x00.
    1. The `config_read_trig` register on the host FPGA is set to 0x01,
    triggering configuration transmission by the firmware.
    1. (Firmware) A configuration read is performed by the firmware.
    1. (Firmware) `config_trig` is set to 0x00 by the firmware.
    1. (Firmware) `OE_CONFIGRACK` is pushed onto the signal stream by the
    firmware.
    1. The signal stream is pumped until `OE_CONFIGRACK` is received indicating
    that the host FPGA has completed reading the specified device register and
    copied its value to the `config_reg_value` register.

- _Note:_ Following successful or unsuccessful device register read or write,
  the COBS encoded ACK or NACK packets must be passed to the signal stream.

## Data input channel (32-bit, asynchronous, read-only)
The _data input_ channel provides high bandwidth communication from the FPGA
firmware to the host computer using direct memory access (DMA) via calls to
`oe_read_frame()`. From the host's perspective, its behavior is equivalent to a
read-only, blocking UNIX named pipe with the exception that data can only be
read on 32-bit, instead of 8-bit, boundaries. Read-frames are pushed into the
data input channel at a rate dicated by the FPGA firmware. It is incumbent on
the host to call `oe_read_frame` fast enought to prevent buffer overflow. At
the time of this writing, the data input buffer occupies a 512 MB segment of
kernal RAM. Increased bandwidth demands will nessestate the creation of a
user-space buffer. This change will have no effect on the API.

## Data output channel (32-bit, asynchronous, write-only)
The _data output_ channel provides high bandwidth communication from the host
computer to the FPGA firmware using DMA via calls to `oe_write_frame()`. From
the host's perspective, its behavior is equivalent to a write-only, blocking
UNIX named pipe with the exception that data can only be read on 32-bit,
instead of 8-bit, boundaries. Its performance characteristics are largely
identical to the data input channel.

# `liboepcie`: An Open Ephys++ API Implementation

`liboepcie` is a C library that implements the [Open Ephys++ API
Specification](#api-specification). It is composed to two mutually exclusive
file pairs:

1. oepcie.h and oepcie.c : main API implementation
1. oedevice.h and oedevices.c : officially supported device and register
   definitions

oedevices.h can be ignored for those that do not wish to conform to the
device definition specifications of this project. 

## Exposed Types

### Integer types
- `oe_size_t`: Fixed width size integer type.
- `oe_dev_id_t`: Fixed width device identity integer type.
- `oe_reg_addr_t`: Fixed width device register address integer type.
- `oe_reg_value_t`: Fixed width device register value integer type.

### `oe_ctx`
Opaque handle to a structure which contains hardware and device state
information. There is generally one context per process using the library.

### `oe_opt_t`
Context option enumeration. See the description of `oe_set_opt()` and
`oe_get_opt()` for valid values.

### `oe_device_t`
`oe_device_t` describes one of potentially many pieces of hardware within a
context. Examples include Intan chips, IMU, stimulators, immediate IO circuit,
auxiliary ADC, etc. Each valid device type has a unique ID which is enumerated
in the auxiliary `oedevices.h` file or some use-specific header. A map of
available devices is read from hardware and stored in the current context via a
call to `oe_init_ctx()`.  This map can be examined via calls to `oe_get_opt()`.

``` {.c}
typedef struct {
    oe_dev_id_t id;         // Device ID number 
    oe_size_t read_size;    // Device data read size per frame in bytes
    oe_size_t num_reads;    // Number of frames that must be read to construct a full sample (e.g., for row reads from camera)
    oe_size_t write_size;   // Device data write size per frame in bytes
    oe_size_t num_writes;   // Number of frames that must be written to construct a full output sample
} oe_device_t;
```

### `oe_error_t`
Error code enumeration.

``` {.c}
typedef enum oe_error {
    OE_ESUCCESS         =  0,  // Success
    OE_EPATHINVALID     = -1,  // Invalid stream path, fail on open
    OE_EREINITCTX       = -2,  // Double initialization attempt
    OE_EDEVID           = -3,  // Invalid device ID on init or reg op
    OE_EREADFAILURE     = -4,  // Failure to read from a stream/register
    OE_EWRITEFAILURE    = -5,  // Failure to write to a stream/register
    OE_ENULLCTX         = -6,  // Attempt to call function w null ctx
    OE_ESEEKFAILURE     = -7,  // Failure to seek on stream
    OE_EINVALSTATE      = -8,  // Invalid operation for the current context run state
    OE_EDEVIDX          = -9,  // Invalid device index
    OE_EINVALOPT        = -10, // Invalid context option
    OE_EINVALARG        = -11, // Invalid function arguments
    OE_ECANTSETOPT      = -12, // Option cannot be set in current context state
    OE_ECOBSPACK        = -13, // Invalid COBS packet
    OE_ERETRIG          = -14, // Attempt to trigger an already triggered operation
    OE_EBUFFERSIZE      = -15, // Supplied buffer is too small
    OE_EBADDEVMAP       = -16, // Badly formated device map supplied by firmware
    OE_EBADALLOC        = -17, // Bad dynamic memory allocation
    OE_ECLOSEFAIL       = -18, // File descriptor close failure, check errno
    OE_EDATATYPE        = -19, // Invalid underlying data types
    OE_EREADONLY        = -20, // Attempted write to read only object (register, context option, etc)
    OE_ERUNSTATESYNC    = -21, // Software and hardware run state out of sync
} oe_error_t;
```

## oe_create_ctx
Create a hardware context. A context is an opaque handle to a structure which
contains hardware and device state information, configuration capabilities, and
data format information. It can be modified via calls to `oe_set_opt`. Its
state can be examined by `oe_get_opt`.

``` {.c}
oe_ctx oe_create_ctx();
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
interrupt signal (TOD0: Which?) is raised and any blocking operations will
return immediately. Attached resources (e.g. file descriptors and allocated
memory) are closed and their resources freed.

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

Following a successful call to `oe_init_ctx`, the hardware's acqusition
parameters and run state can be manipulated using calls to `oe_get_opt`.

## oe_get_opt
Get context options. NB: This follows the pattern of
[zmq_getsockopt()](http://api.zeromq.org/4-1:zmq-getsockopt).

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
The `oe_get_opt()` function shall set the option specified by the `option`
argument to the value pointed to by the `value` argument for the context
pointed to by the `ctx` argument. The `size` provides a pointer to the size of
the option value in bytes. Upon successful completion `oe_get_opt` shall modify
the value pointed to by `size` to indicate the actual size of the option value
stored in the buffer.

Following a successful call to `oe_init_ctx()`, the following socket options
can be read:

#### `OE_CONFIGSTREAMPATH`\*
Obtain path/URI specifying config data stream.

| | | |
|-|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_config_32, \\\\.\\xillybus_oe_config_32 (Windows) |

#### `OE_READSTREAMPATH`\*
Obtain path/URI specifying input data stream.

| | | |
|-|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_input_32 \\\\.\\xillybus_oe_input_32 (Windows) |

#### `OE_WRITESTREAMPATH`\*
Obtain path/URI specifying input data stream.

| | | |
|-|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_output_32, \\\\.\\xillybus_oe_output_32 (Windows) |

#### `OE_SIGNALSTREAMPATH`\*
Obtain path/URI specifying hardware signal data stream

| | | |
|-|-|-|
| option value type 	    | char * |
| option value unit 	    | N/A |
| default value     	    | file:///dev/xillybus_oe_signal_8, \\\\.\\xillybus_oe_signal_8 (Windows) |

#### `OE_DEVICEMAP`
Obtain the device map

| | | |
|-|-|-|
| option value type 	    | oe_device_t * |
| option value unit 	    | Pointer to a pre-allocated array of `oe_device_t` structs |
| default value     	    | N/A |

#### `OE_NUMDEVICES`
Number of devices in the device map

| | | |
|-|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | N/A |
| default value     	    | N/A |

#### `OE_READFRAMESIZE`
Size of a read frame (sample) in bytes

| | | |
|-|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | N/A |
| default value     	    | N/A |

#### `OE_WRITEFRAMESIZE`
Size of a write frame (sample) in bytes

| | | |
|-|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | N/A |
| default value     	    | N/A |

#### `OE_RUNNING`
Hardware run state

| | | |
|-|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | True or False (i.e. 0 or > 0) |
| default value     	    | False |

#### `OE_SYSCLKHZ`
System clock frequency

| | | |
|-|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | Hz |
| default value     	    | 100e6 |

## oe_set_opt
Set context options. NB: This follows the pattern of
[zmq_setsockopt()](http://api.zeromq.org/4-1:zmq-setsockopt).

``` {.c}
int oe_set_opt(oe_ctx ctx, const oe_opt_t option, const void* value, size_t size);
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
The `oe_set_opt` function shall set the option specified by the `option`
argument to the value pointed to by the `value` argument within `ctx`. The
`size` indicates the size of the `value` in bytes.

The following socket options can be set:

#### `OE_CONFIGSTREAMPATH`\*
Set path/URI specifying config data stream.

| | | |
|-|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_config_32, \\\\.\\xillybus_oe_config_32 (Windows) |

#### `OE_READSTREAMPATH`\*
Set path/URI specifying input data stream.

| | | |
|-|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_input_32 \\\\.\\xillybus_oe_input_32 (Windows) |

#### `OE_WRITESTREAMPATH`\*
Set path/URI specifying input data stream.

| | | |
|-|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_output_32, \\\\.\\xillybus_oe_output_32 (Windows) |

#### `OE_SIGNALSTREAMPATH`\*
Set path/URI specifying hardware signal data stream

| | | |
|-|-|-|
| option value type 	    | char * |
| option value unit 	    | N/A |
| default value     	    | file:///dev/xillybus_oe_signal_8, \\\\.\\xillybus_oe_signal_8 (Windows) |

#### `OE_RUNNING`\*\*
Set/clear master clock gate. Any value greater than 0 will start acqusition.
Writing 0 to this option will stop acqusition, but will not reset context
options or the sample counter.

| | | |
|-|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | True or false (i.e. 0 or not 0) |
| default value     	    | False |

#### `OE_RESET`\*\*
Trigger global hardware reset. Any value great than 0 will trigger a hardware
reset. In this case, acquisition is stopped and all global hardware parameters
(clock multipliers, sample counters, etc) are defaulted.

| | | |
|-|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | Trigger (i.e. any value greater than 0) |
| default value     	    | Untriggered |

\* Invalid following a successful call to `oe_init_ctx()`. Before this, will
return with error code `OE_EINVALSTATE`.

\*\* Invalid until a successful call to `oe_init_ctx()`. After this, will
return with error code `OE_EINVALSTATE`.

## oe_read_reg
Read a configuration register on a specific device.

``` {.c}
int oe_read_reg(const oe_ctx ctx, const oe_dev_idx_t dev_idx, const oe_reg_addr_t addr, oe_reg_val_t *value)
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
int oe_write_reg(const oe_ctx ctx, const oe_dev_idx_t dev_idx, const oe_reg_addr_t addr, const oe_reg_val_t value)
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

## oe_read
Read high-bandwidth input data stream.

``` {.c}
int oe_read_frame(const oe_ctx ctx, oe_frame_t **frame)
```

### Arguments
- `ctx` context
- `frame` Pointer to a `oe_rframe_t` pointer

### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### Description
`oe_read_frame` allocates and populates a `struct` corresponding to a single
[read-frame](#frame) from the data input channel into host memory.

``` {.c}
// Read frame type
typedef struct oe_rframe {
    uint64_t sample_no;   // Sample no.
    uint16_t num_dev;     // Number of devices in frame
    uint8_t corrupt;      // Is this frame corrupt?
    oe_size_t *dev_idxs;  // Array of device indicies in frame
    size_t *dev_offs;     // Device data offsets within data block
    uint8_t *data;        // Multi-device raw data block

} oe_rframe_t;
```

## oe_write_frame
NB: Not implemented in first version
Write a frame to the output data channel.

``` {.c}
int oe_write_frame(const oe_ctx ctx, oe_frame_t *frame)
```

### Arguments
- `ctx` context
- `frame` an [output frame]()

### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### Description
`oe_write_frame` writes a pre-allocated and populated `stuct` corresponding to
a single [write-frame](#frame) into the asynchronous data output channel from
host memory.

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

## oe_error_str
convert an error number from `oe_error_t` into a human readable string.

``` {.c}
const char *oe_error_str(oe_error_t err)
```

### arguments
- `err` error code

### returns `const char *`
Pointer to an error message string

## oe_device_str
Convert an number from `oe_device_id_t` into human readable string.

``` {.c}
const char *oe_device(oe_device_id_t dev_id)
```

### Arguments
- `dev_id` device id

### Returns `const char *`
Pointer to a device id string
