# liboepcie Specification

## License

[MIT](https://en.wikipedia.org/wiki/MIT_License)

## Purpose
Configure hardware and read and write streaming data from and to a host applications.

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
Implementation shall be in C to facilitate cross platform and cross-language
use.

## Scope and External Dependencies

- This is a low level library used by high-level library and or application
  plugin developers. It is not meant to be used by neuroscientists directly.

- The only external dependency aside from the c standard library is
  [Xillybus](http://xillybus.com/), which will be used for PCIe communication.
  Because xillybus provides abstracts this functionality to the level of file
  IO, a drop in replacement without its commercial restrictions is possible if
  open-ephys ever gathers the funds to do so.

- The public programming interface, supported device ID enumeration, and public
  type declarations should be limited to a single header file.

## Types and Definitions

#### Device
A _device_ (`oe_device_t`) is defined as configurable piece of hardware with
its own register address space (e.g. an integrated circuit) or something
programmed within the firmware to emulate this (e.g. a MUX or microcontroller
implemented in the FPGA's logic). On the FPGA firmware, each device corresponds
to a module with several standard methods (TODO: Elaborate on this!). Host
interaction with a device is facilitated using a description, which is provided
by a `struct` with the following elements:

1. `enum config_device_id`: Device ID number which is globally enumerated for the
   entire project
    - e.g. Intan RHD2032 could be 0, Intan RHD2064 could 1, etc.
    - e.g. MWL001GPIOMUX is a MUX implemented in the headstage FPGA that is
      used to route SERDES GPIO lines
    - This enum grows with the number of devices supported by the library.
      There is a single enum for the entire library which enumerates all
      possible devices that are controlled across `context` configurations.
    - Each `context` is only responsible for controlling a subset of all
      supported devices.

    ```
    typedef enum device_id {
        RHD2032,
        RHD2064
        MPU9250,
        MWL001GPIOMUX,
        ...
    } oe_device_id_t
    ```

2. `int read_offset`: Byte count offset within data packet that this
   device's data can be found
    - -1 indicates that no data is sent by the device
3. `size_t read_size`:  bytes of each transmitted data packet from this device.
    - 0 indicates that it does not send data.
4. `int write_offset`: Byte count offset within data packet that this
   device's write data can be written
    - -1 indicates that no data can be written to the device
5. `size_t write_size`:  bytes within the output packet transmitted data packet
   to this device.
    - 0 indicates that it does not send data.

So, a `device` struct looks like this:

``` {.c}
struct device {
    device_id_t id;
    int         read_offset;
    size_t      read_size;
    int         write_offset;
    size_t      write_size;
};
```

#### Context

A _context_ (`oe_ctx`) holds all required configuration and state information for a
devices associated with one PCIe bus in the following hierarchy:

1. `struct stream_fids`: communication stream file descriptor structure
    - `char * header_path`:  Header stream path
    - `int header_fid`:  Header stream file descriptor
    - `char * config_path`:  Config stream path
    - `int config_fid`:  Config stream file descriptor
    - `char * data_in_path`:  Data input stream path
    - `int data_in_fid`:  Data input stream file descriptor
2. `struct dev_map` : Device map
    - `int num_dev`: Number of devices in this map
    - `device devs[num_dev]`: Array of device object handles
        - `struct device`: device object (see [Device](#device))
        - The device's index within this array is how it is accessed in the
          function calls specified below.

State details should be hidden in implementation file (.c). Pointer to opaque
type is exposed publicly in header (.h)

```
// .h
typedef struct oe_ctx_impl *oe_ctx;
int oe_make_ctx(oe_ctx **ctx);

// .c
struct oe_ctx_impl {
    // state
}
```

API calls will typically take a pointer to a context as the first argument and
use it to look up required state information to enable communication, or to add
there own information to the context (e.g. add device information).

```
int oe_api_function(oe_ctx *ctx, ...);
```

#### FPGA/Host Communication Channels
The library must supports communication over the following channels

1. Signal channel (asynchronous, read only)
    - Provides a way for the firmware to inform host of configuration results and available devices
    - Communicates with host using a
      [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing)
      stream.
    - Should generally be used on a separate thread from high-bandwidth communication

2. Configuration channel (synchronous, read and write)
    - Supports seeking to, reading, and writing the following registers:

        - `uint32_t config_device_id`: Device ID register. Specify a device endpoint as
          enumerated by the firmware (e.g. an Intan chip, or a IMU chip)  and
          to which communication will be directed using `config_reg_addr` and
          `config_reg_value`.

        - `uint32_t config_reg_addr`: register address of configuration to be
          written

        - `int32_t config_reg_value`: configuration value to be written to or
          read from and that corresponds to `config_reg_addr` on device
          `config_device_id`

        - `uint8_t config_rw`: bit indicating if a read or write should be
          performed. 0 indicates read operation. 1 indicates write operation.

        - `uint8_t config_trig`: set high to trigger either register read or
          write operation depending on the state of `config_rw`. If `config_rw`
          is 0, a read is performed. In this case `config_reg_value` is updated
          with value stored at `config_reg_addr` on device at
          `config_device_id`. If the specified register is not readable,
          `config_reg_value` is populated with a magic number (`OE_REGMAGIC`).
          If `config_rw`is 1, `config_reg_value` is written to register at
          `config_reg_addr` on device `config_device_id`. The `config_trig`
          register shall always be set low by the firmware following
          transmission even if it is not successful or does not make sense
          given the address register values.

    - Appropriate values of `config_reg_addr` and `config_reg_value` are determined by:
        - Looking at a device's data sheet if the device is an integrated circuit
        - Looking at the verilog source code comments if the device exists as a
          module that is implemented within the FPGA (e.g a MUX) or as a
          verilog module that abstracts a PCB or sub-circuit (e.g. a
          digital IO port).

    - `config_reg_addr` and `config_reg_value` are always 32-bit unsigned
      integers. A device module must implement methods for appropriately
      translatting these values into data that is properly fomatted for a a
      particular device (e.g. changing endianness or removing significant
      bits).

    - Registers in the configuration stream are ordered in accordance with the
      above enumeraiton. `SEEK` loations, relative to the start of the stream,
      are provided by the `oe_config_reg` enum.

3. Data input channel (asynchronous, read-only)
    - High-bandwidth communication channel from firmware to host.
    - FIFO is filled with untyped data once per master clock cycle
    - Packet size is determined by the attached device IDs and the device Data Map

4. TODO: Data output channel (asynchronous, write-only)
    - High-bandwidth  communication channel from host to firmware
    - FIFO is filled with untyped data once per master clock cycle

## API Spec

### Types

#### Integer types
- `oe_dev_idx_t`: Fixed width device index integer type.
- `oe_dev_id_t`: Fixed width device identity integer type.
- `oe_reg_addr_t`: Fixed width device register address integer type.
- `oe_reg_value_t`: Fixed width device register value integer type.
- `oe_device_t

#### `oe_ctx`
Opaque handle to a structure which contains hardware and device state
information. There is generally one context per process using the library.

#### `oe_opt_t`
Context option enumeration. See `oe_set_opt` and `oe_get_opt` for valid values.

#### `oe_device_id_t`
Device identiy enumeration. Provides agreed-upon, liboepcie-specific IDs for
all devices supported by the firmware.

``` {.c}
typedef enum oe_device_id {
    OE_IMMEDIATEIO = 0,
    OE_RHD2032,
    OE_RHD2064,
    OE_MPU9250,
} oe_device_id_t;
```

#### `oe_device_t`
One of potentially many pieces of hardware within a context. Examples include
Intan chips, IMU, stimulators, immediate IO circuit, auxiliary ADC, etc. Each
valid device type has a unqique ID which is enumerated in `oe_device_id_t`. A
map of available devices is stored in the current context via `oe_init_ctx`.
This map can be examined via calls to `oe_get_opt`.

``` {.c}
typedef struct oe_device {
    oe_dev_id_t id;
    oe_size_t read_offset;
    oe_size_t read_size;
    oe_size_t write_offset;
    oe_size_t write_size;
} oe_device_t;
```

#### `oe_error_t`
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

### oe_create_ctx
Create a hardware context. A context is an opaque handle to a structure which
contains hardware and device state information, configuration capabilities, and
data format information. It can be modified via calls to `oe_set_opt`. Its
state can be examined by `oe_get_opt`.

``` {.c}
oe_ctx oe_create_ctx();
```

#### Returns `oe_ctx`
- An opaque handle to the newly created context if successful. Otherwise it
  shall return NULL and set errno to `EAGAIN`

#### Description
On success a context struct is allocated and created, and its handle is passed
to the user. The context holds all state used to communicate with a set of
hardware devices. It holds paths to FIFOs and configuration communication
channels and knowledge of the hardware's run state. It is configured through
calls to `oe_set_opt`. It can be examined through calls to `oe_get_opt`.

### oe_destroy_ctx
Terminate a context and free bound resources.

``` {.c}
int oe_destroy_ctx(oe_ctx ctx)
```

#### Arguments
- `ctx` context

#### Returns `int`
- O: success
- Less than 0: `oe_error_t`

#### Description
During context destruction, all resources allocated by `oe_create_ctx` are
freed. This function can be called from any context run state. When called, an
interrupt signal (TODO: Which?) is raised and any blocking operations will
return immediately. Attached resources (e.g. file descriptors and allocated
memory) are closed and their resources freed.

### oe_init
Initialize a context, opening all file streams etc.

``` {.c}
int oe_init_ctx(oe_ctx *c)
```

#### Arguments
- `c` context

#### Returns `int`
- O: success
- Less than 0: `oe_error_t`

#### Description
Upon a call to `oe_init_ctx`, the following actions take place

1. All required data streams are opened.
2. A device map is read from the firmware. It can be examined via calls t
   `oe_get_opt`.

Following a successful call to `oe_init_ctx`, the hardware's acqusition
parameters and run state can be manipulated using calls to `oe_get_opt`.

### oe_get_opt
Get context options. NB: This follows the pattern of
[zmq_getsockopt()](http://api.zeromq.org/4-1:zmq-getsockopt).

``` {.c}
int oe_get_opt(const oe_ctx ctx, const oe_opt_t option, void* value, size_t *size);
```

#### Arguments
- `ctx` context to read from
- `option` option to read
- `value` buffer to store value of `option`
- `size` pointer to the size of `value` (including terminating null character,
  if applicable) in bytes

#### Returns `int`
- O: success
- Less than 0: `oe_error_t`

#### Description
The `oe_get_opt`() function shall set the option specified by the `option`
argument to the value pointed to by the `value` argument for the context
pointed to by the `ctx` argument. The `size` provides a pointer to the size of
the option value in bytes. Upon successful completion `oe_get_opt` shall modify
the value pointed to by `size` to indicate the actual size of the option value
stored in the buffer.

Following a succesful call to `oe_init`, the following socket options can be read:

|`OE_CONFIGSTREAMPATH`  	| URI specifying config data stream. |
|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_config_32 |

|`OE_DATASTEAMPATH`  	    | URI specifying input data stream.  |
|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_input_32 |

|`OE_SIGNALSTREAMPATH`  	| URI specifying hardware signal data stream |
|-|-|
| option value type 	    | char * |
| option value unit 	    | N/A |
| default value     	    | file:///dev/xillybus_oe_signal_32 |

|`OE_DEVICEMAP`             | Obtain the device map |
|-|-|
| option value type 	    | oe_device_t * |
| option value unit 	    | Pointer to a pre-allocated array of `oe_device_t` structs |
| default value     	    | N/A |

|`OE_NUMDEVICES`            | Number of devices in the device map |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | N/A |
| default value     	    | N/A |

|`OE_READFRAMESIZE`         | Size of a read frame (sample) in bytes |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | N/A |
| default value     	    | N/A |

|`OE_WRITEFRAMESIZE`        | Size of a write frame (sample) in bytes |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | N/A |
| default value     	    | N/A |

|`OE_RUNNING`               | Hardware run state |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | True or False (i.e. 0 or > 0) |
| default value     	    | False |

|`OE_SYSCLKHZ`              | System clock frequency |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | Hz |
| default value     	    | 100e6 |

|`OE_FSCLKHZ`               | Sample clock frequency |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | Hz |
| default value     	    | 30e3 |

|`OE_FSCLKM`                | Sample clock frequency multiplier|
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | N/A |
| default value     	    | 3 |

|`OE_FSCLKD`                | Sample clock frequency divider|
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | N/A |
| default value     	    | 10000 |

### oe_set_opt
Set context options. NB: This follows the pattern of
[zmq_setsockopt()](http://api.zeromq.org/4-1:zmq-setsockopt).

``` {.c}
int oe_set_opt(oe_ctx ctx, const oe_opt_t option, const void* value, size_t size);
```

#### Arguments
- `ctx` context
- `option` option to set
- `value` value to set `option` to
- `size` length of `value` in bytes

#### Returns `int`
- O: success
- Less than 0: `oe_error_t`

#### Description
The `oe_set_opt` function shall set the option specified by the `option`
argument to the value pointed to by the `value` argument within `ctx`. The
`size` indicates the size of the `value` in bytes.

The following socket options can be set:

|`OE_CONFIGSTREAMPATH`\*  	| Set URI specifying config data stream. |
|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_config_32 |

|`OE_DATASTEAMPATH`\*  	    | Set URI specifying input data stream.  |
|-|-|
| option value type         | char * |
| option value unit         | N/A |
| default value             | file:///dev/xillybus_oe_input_32 |

|`OE_SIGNALSTREAMPATH`\*  	| Set URI specifying hardware signal data stream |
|-|-|
| option value type 	    | char * |
| option value unit 	    | N/A |
| default value     	    | file:///dev/xillybus_oe_signal_32 |

|`OE_RUNNING`\*\*  	        | Set/clear master clock gate |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | True or false (i.e. 0 or not 0) |
| default value     	    | False |
Any value greater than 0 will start acqusition. Writing 0 to this option will stop acqusition, but will not reset context options or the sample counter.

|`OE_RESET`\*\*  	        | Trigger global hardware reset |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | Trigger (i.e. any value greater than 0) |
| default value     	    | Untriggered |
Any value great than 0 will trigger a hardware reset. In this case, acqusition
is stopped and all global hardware parameters (clock multipliers, sample
counters, etc) are defaulted.

|`OE_FSCLM`\*\*  	        | Set sample clock multiplier |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | Sample clock ratio numerator |
| default value     	    | 3 |
Used to derive the sample clock frequency from the system clock. The sample
clock frequncy = M * system clock frequency / D;

|`OE_FSCLD`\*\*  	        | Set sample clock divider |
|-|-|
| option value type 	    | oe_reg_val_t |
| option value unit 	    | Sample clock ratio denominator |
| default value     	    | 10000 |
Used to derive the sample clock frequency from the system clock. The sample
clock frequncy = M * system clock frequency / D;

\* Invalid following a successful call to `oe_init()`. In this case, will return with error code `OE_EINVALSTATE`.
\*\* Invalid until a succesful call to `oe_init()`. In this case, will return with error code `OE_EINVALSTATE`.

### oe_read_reg
Read a configuration register on a specific device.

``` {.c}
int oe_read_reg(const oe_ctx ctx, const oe_dev_idx_t dev_idx, const oe_reg_addr_t addr, oe_reg_val_t *value)
```

#### Arguments
- `ctx` context
- `dev_idx` physical index number
- `addr` Thekey of register to write to
- `value` pointer to an int that will store the value of the register at `addr` on `dev_idx`

#### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

#### Description
Upon a call to `oe_write_reg`, the following actions take place:

1. The value of `config_trig` is checked.
    - If it is 0x00, the function call proceeds.
    - Else, the function call returns with OE_ERETRIG.
1. `dev_idx` is copied to the `config_device_id` register on the host FPGA.
1. `addr` is copied to the `config_reg_addr` register on the host FPGA.
1. The `config_rw` register on the host FPGA is set to 0x00.
1. The `config_read_trig` register on the host FPGA is set to 0x01, triggering
   configuration transmission by the firmware.
1. (Firmware) A configuration read is performed by the firmware.
1. (Firmware) `config_trig` is set to 0x00 by the firmware.
1. (Firmware) `OE_CONFIGRACK` is pushed onto the signal stream by the firmware.
1. The signal stream is pumped until `OE_CONFIGRACK` is received indicating
   that the host FPGA has completed reading the specified device register and
   copied its value to the `config_reg_value` register.

### oe_write_reg
Set a configuration register on a specific device.

``` {.c}
int oe_write_reg(const oe_ctx ctx, const oe_dev_idx_t dev_idx, const oe_reg_addr_t addr, const oe_reg_val_t value)
```

#### Arguments
- `ctx` context
- `dev_idx` the device index to read from
- `addr` register address within the device specified by `dev_idx` to write
  to
- `value` value with which to set the register at `addr` on the device
  specified by `dev_idx`

#### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

#### Description
Upon a call to `oe_write_reg`, the following actions take place:

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
1. (Firmware) `OE_CONFIGWACK` is pushed onto the signal stream by the firmware.
1. The signal stream is pumped until `OE_CONFIGWACK` is received indicating
   that the host FPGA has attempted to write to the specified device register.

Note that successful return from this function does not guarantee that the
register has been properly set. Confirmation of the register value can be made
using a call to `oe_read_reg`.

### oe_read
Read high-bandwidth input data stream.

``` {.c}
int oe_read(const oe_ctx ctx, void *data, size_t size)
```

#### Arguments
- `ctx` context
- `data` buffer to read data into
- `size` size of buffer in bytes

#### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

#### Description
`oe_read` reads data from a asynchronous input buffer into host memory. The
layout of retrieved data is determined by a particular hardware configuration
and corresponding device map. Each data frame corresponds to a round-robin
through all data producing devices in the device map. The read byte offsets of each
device within a data frame can be found by examining the device map via calls
to `oe_get_opt`.

### oe_write
NB: Not implemented in first version
Write data to an open stream

``` {.c}
int oe_write(const oe_ctx ctx, void *data, size_t size)
```

#### Arguments
- `ctx` context
- `data` buffer to read data into
- `size` size of buffer in bytes

#### Returns `int`
- Less than 0: `oe_error_t`

#### Description
`oe_write` writes data into an asynchronous output stream from host memory. The
layout of output data is determined by a particular hardware configuration, and
corresponding device map. Data placed into the stream corresponds to a
round-robin through all data accepting devices in the device map. The wite byte
offsets of each device within a data frame can be found by examining the device
map via calls to `oe_get_opt`.

### oe_error
Convert an instance of `oe_error_t` into a human readable string.

``` {.c}
void oe_error(oe_error_t err, char *str, size_t size);
```

#### Arguments
- `err` error code
- `str` string buffer
- `size` size of string buffer in bytes

#### Returns `int`
- 0: success
- Less than 0: `oe_error_t`

### oe_error
Convert an instance of `oe_device_id_t` into human readable string.

``` {.c}
int oe_device(const oe_device_id_t dev_id, char *str, size_t size);
```

#### Arguments
- `dev_id` device id
- `str` string buffer
- `size` size of string buffer in bytes

#### Returns `int`
- 0: success
- Less than 0: `oe_error_t`
