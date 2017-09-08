# liboepcie Specification

## License

[MIT](https://en.wikipedia.org/wiki/MIT_License)

## Purpose
Configure hardware and read stream data from a host applications.

## Design Requirements
- Low latency (sub millisecond)
- High bandwidth (> 1000 neural data channels)
- Support for different hardware components on a single PCIe bus
    - Support generic mixes of hardware elements
    - Generic configuration channels
    - Generic data input stream
    - Generic data output stream
- Useable by different host applications
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

- The public programming interface and public type declarations should be
  limited to one c header. Device map structures should contained in a separate
  header file.

## Types and Definitions

#### Device
A `device` is defined as configurable piece of hardware with its own register
address space (e.g. an integrated circuit) or something programmed within the
firmware to emulate this (e.g. a MUX or microcontroller implemented in the
FPGA's logic). On the FPGA firmware, each device corresponds to a module with
several standard methods (TODO: Elaborate on this!). Host interaction with a
device is facilitated using a description, which is provided by a `struct` with
the following elements:

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
6. ~~`char * read_desc`: [numpy dtype](https://docs.scipy.org/doc/numpy/reference/generated/numpy.dtype.html) description of data within packet which specifies casting requirements.~~
    - Instead of this, human readable documentation about the layout of data
      associated with a particular device should be specified in the header
      file.

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

A _context_ holds all required configuration and state information for a
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

1. Header channel (synchronous, read only) (NOTE: This may not be implemented
   on the first go since it's function can be accomplished by hard coding a
   configuration map on the host side.)

    1. 0, int32: is configuration ID, which should correspond to a struct ID in the c library
    2. 4, int32: Device ID, which is a particular configurable element
      within a configuration. This could be an Intan Chip, an IMU, or a
      microcontroller. -1 indicates end of header.
    3. 8, int32: Device ID, which is a particular configurable element
      within a configuration. This could be an Intan Chip, an IMU, or a
      microcontroller. -1 indicates end of header.
    4. 12, int32: Specifies number of bytes to read (N) in configuration
      description string
    5. Repease starting at 2.

2. Configuration channel (synchronous, read and write)
    - Supports seeing to, reading, and writing the following registers:

        - `input [31:0] config_device_id`: Device ID register. Specify a device endpoint as
          enumerated by the firmware (e.g. an Intan chip, or a IMU chip)  and
          to which communication will be directed using `config_reg_addr` and
          `config_reg_value`.

        - `input [31:0] config_reg_addr`: register address of configuration to be
          written

        - `input [31:0] config_reg_value`: configuration value to be written to
          `config_reg_addr` on device `config_device_id`

        - `input config_write_trig`: set high to trigger write of `config_reg_value` to
          `config_reg_addr` on device `config_device_id`. This register is always set low by the
          firmware following transmission even if it is not successful or does not
          make sense given the address register values.

        - `output [31:0] config_write_ack`: Write acknowledgement register. Set to
          1 on successful tranmission of `config_reg_value`. Set to 2 on
          unsuccessful tranmission. Must be set to 0 by host at start of write
          handshake.

        - `output [31:0] config_read_value`: Register holds value of stored at
          `config_reg_addr` on device at `config_device_id` after a configuration
          read has taken place by triggering `config_read_trig`.

        - `input config_read_trig`: Read trigger. Causes `config_reg_value` to be
          updated with value stored at `config_reg_addr` on device at
          `config_device_id`. If the specified register is
          not readable, `config_reg_value` is populated with a magic number (123456).

        - `output [31:0] config_read_ack`: Read acknowledgement register. Set to
          1 on successful setting of `config_reg_value`. Set to 2 on
          unsuccessful setting. Must be set to 0 by host at start of write
          handshake.

    - Appropriate values of `config_reg_addr` and `config_reg_value` are determined by:
        - Looking at a device's data sheet if the device is an integrated circuit
        - Looking at the verilog source code comments if the device exists as a
          module that is implemented within the FPGA (e.g a MUX) or as a
          verilog module that abstracts a PCB or sub-circuit (e.g. a
          digital IO port).

    - `config_reg_addr` and `config_reg_value` are always 32-bit unsigned
      integers. A device module must implement methods for appropriately
      translatting these values into data that is properly fomatted for a a
      particular device (e.g. chaning endianness or removing significant
      bits).

3. Data input channel (asynchronous, read-only)
    - High-bandwidth communication channel from firmware to host.
    - FIFO is filled with untyped data once per master clock cycle
    - Packet size is determined by the attached device IDs and the device Data Map

4. TODO: Data output channel (asynchronous, write-only)
    - High-bandwidth  communication channel from host to firmware
    - FIFO is filled with untyped data once per master clock cycle

## API Spec

### Definitions and Relations

- TODO: Dependency/hierarchy graph
- TODO: This section is not finished and will likely be removed

- context: opaque handle to a structure which contains hardware and device
  state information. There is generally one context per process using the
  library.
- device: one of potentially many pieces of hardware within a context.
  Examples: headstage, digital IO board, analog IO board, etc.
- stream: a structure specifying parameters of a data stream.  A data stream is
  a "high bandwidth" connection between the host application and potentially
  many devices. The data associated with a particular device ends up as a set
  of bytes in a buffer that is sent to the device (output stream) or is
  acquired from the hardware (input stream). The location of these bytes within
  the stream is specified via the devices `data_map` and `data_size` members.
- configuration: A set of integers defining a configuration setting on a device.

### oe_create_ctx
Create a hardware context. A context is an opaque handle to a structure which
contains hardware and device state information, configuration capabilities, and
data format information. Initially, it is populated by reasonable default
state. It can then be further configured by calls to `oe_setopt`.

``` {.c}
oe_ctx *oe_create_ctx();
```

#### Returns `oe_ctx *`

- An opaque handle to the newly created context if successful. Otherwise it
  shall return NULL and set errno to `EAGAIN`

#### Description

During successful context creation the following actions take place

1.  TODO

### oe_destroy_ctx

Terminate a context and free bound resources. All blocking calls with exit
immediately.

``` {.c}
int oe_destroy_ctx(oe_ctx *c)
```

#### Arguments
- `c` context

#### Returns `int`

- O: success
- Less than 0: `oe_error_t`

#### Description

Context termination is performed in the following steps:

1. An interrupt signal (TODO: Which?) is raised and any blocking operations will return immediately
2. Attached resources (e.g. file descriptors and allocated memory) are closed and released

### oe_set_option
Set context options. NB: This follows the pattern of
[zmq_setsockopt()](http://api.zeromq.org/4-1:zmq-setsockopt).

``` {.c}
int oe_set_option(oe_ctx *c, int option_name, const void * option_value, size_t option_size)
```

#### Arguments
- `c` context
- `option_name` name of option to set
- `option_value` value to set `option_name` to
- `option_size` length of `option_value` in bytes

#### Description
The `oe_set_option`() function shall set the option specified by the
`option_name` argument to the value pointed to by the `option_value` argument
for the context pointed to by the `c` argument. The `option_size` argument is
the size of the option value in bytes.

The following socket options can be set:

|`OE_HEADERSTREAMPATH`\*  	| Set URI specifying hardware header data stream |
|-|-|
| option value type 	    | char * |
| option value unit 	    | N/A |
| default value     	    | file:///dev/xillybus_oe_header_32 |

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

\* Invalid after a call to `oe_init()`. Will return with error code `OE_ECANTSET`.

### oe_get_option
Get context options. NB: This follows the pattern of
[zmq_getsockopt()](http://api.zeromq.org/4-1:zmq-getsockopt).

``` {.c}
int oe_get_option(const oe_ctx *c, int option_name, void *option_value, size_t *option_size);
```

#### Arguments
- `c` context to read from
- `option_name` name of option to read
- `option_value` buffer to store value at `option_name`
- `option_size` size of `option_value` (including terminating null character, if applicable) in bytes

#### Description
The `oe_get_option`() function shall set the option specified by the
`option_name` argument to the value pointed to by the `option_value` argument
for the context pointed to by the `c` argument. The `option_size` argument is
the size of the option value in bytes. Upon successful completion
`oe_get_option` shall modify the `option_size` argument to indicate the actual
size of the option value stored in the buffer.

The following socket options can be read:

|`OE_HEADERSTREAMPATH*`  	| Set URI specifying hardware header data stream |
|-|-|
| `option_value` type 	    | NULL-terminated character string |
| `option_value` unit 	    | Pointer to pre-allocated character buffer |
| `option_size` unit        | Length of `option_value` buffer in bytes, returns length of internal address character string |
| default value     	    | file:///dev/xillybus_oe_header_32 |

|`OE_CONFIGSTREAMPATH`  	| Set URI specifying config data stream. |
|-|-|
| `option_value` type 	    | NULL-terminated character string |
| `option_value` unit 	    | Pointer to pre-allocated character buffer |
| `option_size` unit        | Length of `option_value` buffer in bytes, returns length of internal address character string |
| default value     	    | file:///dev/xillybus_oe_config_32 |

|`OE_DATASTEAMPATH`  	    | Set URI specifying input data stream.  |
|-|-|
| `option_value` type 	    | NULL-terminated character string |
| `option_value` unit 	    | Pointer to pre-allocated character buffer |
| `option_size` unit        | Length of `option_value` buffer in bytes, returns length of internal address character string |
| default value     	    | file:///dev/xillybus_oe_input_32 |

|`OE_DEVREADOFFSET`\*\*     | Obtain the read-offset value for a particular device index. |
|-|-|
| `option_value` type 	    | device_t * |
| `option_value` unit 	    | Pointer to a pre-allocated `device_t` struct |
| `option_size` unit        | Device index to examine, returns the maximal device index in the configuration |
| default option value      | N/A |

\*\* Invalid until a call to `oe_init()`

### oe_init
Initialize a context, opening all file streams etc.

``` {.c}
int oe_init(oe_ctx *c)
```

#### Arguments
- `c` context

#### Returns `int`
- Less than 0: `oe_error_t`

#### Description
Upon a call to `oe_init`, the following actions take place

1. All required data streams are opened.
2. The hardware configuration is read from the header stream, it can be
   retrieved using `oe_get_option`

### oe_write_reg
Set a configuration register on a specific device.

``` {.c}
int oe_write_reg(const oe_ctx *c, size_t dev_idx, int reg_addr, int reg_value, int *ack);
```

#### Arguments
- `c` context
- `dev_idx` the device index read from
- `reg_addr` register address within the device specified by `dev_idx` to read
- `reg_value` buffer for to place the value stored at `reg_addr` on the
  selected device
- `ack` set to the acknowledgement return code by the host FPGA
  following transmission

#### Returns `int`
- Less than 0: `oe_error_t`

#### Description
Upon a call to `oe_write_reg`, the following actions take place

1. The `config_write_ack` register is set to 0x0000 on the host FPGA.
1. `dev_idx` is used to set the `config_device_id` register on the host FPGA.
1. `reg_addr` is used to set the `config_reg_addr` register on the host FPGA.
1. `reg_value` is used to set the `config_reg_value` register on the host FPGA.
1. The `config_write_trig` register is set to high, triggering configuration transmission.
1. The call blocks until `config_write_ack` is set to a value other than 0x0000
   by the host FPGA.
1. The function returns, setting `ack` to the current `config_write_ack` value.

### oe_read_reg
Read a configuration register from a device on a connected index.

``` {.c}
int oe_read_reg(const oe_ctx *c, size_t dev_idx, int reg_addr, int reg_value, int *ack);
```

#### Arguments
- `c` context
- `index` physical index number
- `key` key of register to write to
- `value` value to write to register
- `mask` bit mask applied to value before it is written

#### Returns `int`
- Less than 0: `oe_error_t`

#### Description
Upon a call to `oe_write_reg`, the following actions take place

1. The `config_read_ack` register is set to 0x0000 on the host FPGA.
1. `dev_idx` is used to set the `config_device_id` register on the host FPGA.
1. `reg_addr` is used to set the `config_reg_addr` register on the host FPGA.
1. `reg_value` is used to set the `config_reg_value` register on the host FPGA.
1. The `config_read_trig` register is set to high, the host firmware to write
   the target register into `config_reg_value`
1. The call blocks until `config_read_ack` is set to a value other than 0x0000
   by the host FPGA.
1. The function returns, setting `ack` to the current `config_read_ack` value.

### oe_read
Read high-bandwidth input data stream.

``` {.c}
int oe_read_(const oe_ctx *c, void *data, size_t *size)
```

#### Arguments
- `c` context
- `data` buffer to read data into
- `size` size of buffer in bytes

#### Returns `int`
- Less than 0: `oe_error_t`

#### Description
`oe_read` reads data from a asynchronous input buffer into host memory. The
layout of retrieved data is determined by a particular hardware configuration,
which is queried using the `oe_header_read` function.  On successful read,
`oe_read` sets `size` to the actual number of bytes read into the data buffer.

### oe_write
NB: Not implemented in first version
Write data to an open stream

``` {.c}
int oe_read_(const oe_ctx *c, void *data, size_t *size)
```

#### Arguments
- `c` context
- `data` buffer to read data into
- `size` size of buffer in bytes

#### Returns `int`
- Less than 0: `oe_error_t`

#### Description
`oe_read` writes data into an asynchronous output stream from host memory. The
layout of output data is determined by a particular hardware configuration,
which is queried using the `oe_header_read` function.  On successful write,
`oe_write` sets `size` to the actual number of bytes written into the output
buffer.

## Public Types

### oe_ctx
TODO

### oe_device
``` {.c}
typedef enum device_id {
    RHD2032,
    RHD2064
    MPU9250,
    MWL001GPIOMUX,
    ...
} oe_device_id_t;
```

``` {.c}
struct oe_device {
    device_id_t id;
    int         read_offset;
    size_t      read_size;
    int         write_offset;
    size_t      write_size;
};
```

### oe_error_t
``` {.c}
typedef enum error {
    OE_ETERMATE,
    OE_EATTEMPT_WRITE_TO_INPUT_STREAM,
    OE_EATTEMPT_READ_FROM_OUTPUT_STREAM,
    OE_ECONTEXT_DOES_NOT_EXIST,
    OE_ECANTSET
} oe_error_t;
```
