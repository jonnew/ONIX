__Open Ephys++__ is hardware, firmware, communication protocols,
specifications, and APIs for serialized, very-high channel count, closed-loop
electrophysiology. It is an evolution of the hardware and software introduced
in [open ephys project](http://www.open-ephys.org/) and involves many of the
same developers. The firmware and API are general purpose -- they can be used
to acquire from and control custom headstages with arbitrary arrangements of
sensors and actuators (e.g. cameras, bioamplifier chips, LED drivers, etc.) and
are not limited to the hardware in this repository.

This project is currently maintained by [jonnew](https://github.com/jonnew).

__Citing this work__: TODO

## Features
- Firmware and API permit acquisition and control of arbitrary arrangements of
  sensors and actuators:

    - Miniscopes
    - Headstages
    - Photometry systems
    - Etc. 

- Submillisecond round-trip communication from brain, through host PC's main
  memory, and back again.
- Flagship headstages:

    - 64-, 128-, 256-channels of electrophysiology
    - Optogenetic stimulation
    - Electrical stimulation
    - 3D-pose measurement
    - Data, user control, and power via a tiny coaxial cable 
    - Wireless communication 

- High-level API language bindings and existing integration with [Open Ephys
  GUI](http://www.open-ephys.org/gui/) and [Bonsai](http://bonsai-rx.org/).
- Quality documentation and easy routes to purchasing assembled devices.

## Repository Contents and Licensing
Each top level directory of this repository corresponds to a distinct system
module. These can be hardware components (e.g. `headstage-64`), or
firmware/software/APIs (e.g. `oepcie`). __Each subdirectory may have distinct
contributors and/or licenses__. Please refer to the README file within each
directory for further information on usage, licensing, etc.

## Hardware
### [eib-64](eib-64/README.md)
64 Channel electrode interface board. Designed for small rodent tetrode
electrophysiology. Works with [headstage-64](./headstage-64/README.md).

### [eib-128](eib-128/README.md)
128 Channel electrode interface board. Designed for large rodent tetrode
electrophysiology. Works with [headstage-256](./headstage-256/README.md).

### [eib-256](eib-256/README.md)
256 Channel electrode interface board. Designed for large rodent tetrode
electrophysiology. Works with [headstage-256](./headstage-256/README.md).

### [headstage-64](headstage-64/README.md)
Serialized, multifunction headstage for small rodents. Supports 64 channels.
Designed to interface with [eib-64](./eib-64/README.md).

### [headstage-256](headstage-256/README.md)
Serialized, multifunction headstage for large rodents. Supports both 128 or 256
channels. Designed to interface with [eib-128](./eib-128/README.md) or
[eib-256](./eib-256/README.md)

### [pcie-host](pcie-host/README.md)
Base board for facilitating PCIe communication, via KC705 or similar, with host
computer. This board fits into an empty PCIe slot and communicates with KC705
via an FMC ribbon cable.

### [nanoz-adapter-64](./nanoz-adapter-64/README.md)
Adapter to interface eib-64 with the popular
[nanoZ](http://www.white-matter.com/nanoz/) electrode impedance tester and
plating device.

### [nanoz-adapter-128-256](./nanoz-adapter-128-256/README.md)
Multiplexed adapter to interface eib-128 and eib-256 with the popular
[nanoZ](http://www.white-matter.com/nanoz/) electrode impedance tester and
plating device.

### [test-board-64](./test-board-64)
Test board for headstage-64. Allows injecting simulated biopotentials into
headstage modules via a selectable passive attenuator. Provides LEDs and
simulated electrical loads for optical and electrical stimulation.

### [test-board-128-256](./test-board-128-256)
Test board for headstage, and headstage-256 modules. Allows injecting simulated
biopotentials into headstage modules via a selectable passive attenuator.
Provides LEDs and simulated electrical loads for optical and electrical
stimulation.

### [headstage-programmer](headstage-programmer/README.md)
JTAG breakout for the [Intel USB Blaster 2](https://www.digikey.com/short/qqw7hm) 
used to program the headstages' MAX10 FPGA.

### pcie-analog-io [WIP]
General purpose analog IO expansion board which communicates with the host
computer via the  sits next to [pcie-host]() board.

## Software
### [liboepcie](oepcie/README.md)
Host API specification and implementation.

## Firmware [WIP]
Binary files for headstage and host FPGAs are available [here](TODO) Firmware
source code is currently available under controlled release. Contact the
maintainer for more information.

