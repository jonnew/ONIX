# Open Ephys++
Hardware, firmware, communication protocols, and APIs for serialized, very-high
channel count, closed-loop electrophysiology.

__Maintainer__: [jonnew](https://github.com/jonnew)

__Note__ This work is a second-order fork. It was based on the following open
source designs:

1. [Intan's headstages](http://intantech.com/index.html): no license provided.
2. [Open Ephys headstages](https://github.com/open-ephys/headstage): Open
   Hardware license. Based upon (1).

__Citing this work__: TODO

## Features
- Integrated electrophysiology (>1000 channels), optogenetic or electrical
  micro stimulation (i.e. no fiber optic tether), 6 DOF pose measurement, etc.
- Submillisecond round-trip communication with host PC's main memory
- Data, user control, and power transmitted over one tiny coaxial cable
- Modular design allows custom integration of individual project components.
- Integrated electrode plating and impedance testing
- Low profile, flat form factor headstage design which minimizes torque
  applied to skull
- High level language bindings and integration with [Open Ephys
  GUI](http://www.open-ephys.org/gui/) and [Bonsai](http://bonsai-rx.org/).
- Quality documentation and easy routes to purchasing assembled devices.

## Repository Contents
Each top level directory of this repository corresponds to a distinct system
module. These can be hardware (e.g. headstage-64), firmware (e.g.
kc705-host-firmware), or software/APIs (e.g. oepcie). Each may have distinct
contributors and/or licenses.  Please refer to the README file withing each
directory for further information.

## Hardware

### [eib-64](eib-64/README.md)
64 Channel electrode interface board. Designed for small rodent tetrode
electrophysiology. Functions with [headstage-64](./headstage-64/README.md).

### [eib-128](eib-128/README.md)
128 Channel electrode interface board. Designed for large rodent tetrode
electrophysiology. Functions with [headstage-256](./headstage-256/README.md).

### [eib-256](eib-128/README.md)
256 Channel electrode interface board. Designed for large rodent tetrode
electrophysiology. Functions with [headstage-256](./headstage-256/README.md).

### [headstage-64](headstage-64/README.md)
Serialized, multifunction headstage for small rodents. Supports 64 ephys
channels. Designed to function with [eib-64](./eib-64/README.md).

### headstage-256 [WIP]
Serialized, multifunction headstage for large rodents. Supports both 128 or 256
channels. Designed to interface with [eib-128](./eib-128/README.md) or
[eib-256](./eib-256/README.md)

### headstage
A low profile 128-channel digital headstage module for amplifying, filtering,
and digitizing microelectrode voltage data from a rat microdrive implant. Up to
256 wires (64 tetrodes) can be acquired by stacking two modules. _Note_ This
design is deprecated and will be removed upon completion of headstage-256.

### serdes-interface
Data serialization board for headstage. Designed for rat tetrode
electrophysiology. _Note_ This design is deprecated and will be removed upon
completion of headstage-256.

### pcie-host
Base board for facilitating PCIe communication, via KC705 or similar, with host
computer. This board fits into an empty PCIe slot and communicates with KC705
via an FMC ribbon cable.

### led-board
Simple board for making pig-tailed, drivable stimulation LEDs for optogenetic
manipulation.

### led-flex-Board
Simple flexible board for making pig-tailed, driveable stimulation LEDs for
optogenetic manipulation using standard chip scale LEDs or direct-attach micro
LEDs.

### nanoz-adapter-64
Adapter to interface eib-64 with the popular
[nanoZ](http://www.white-matter.com/nanoz/) electrode impedance tester and
plating device.

### nanoz-adapter-128-256
Muliplexed adapter to interface eib-128 and eib-256 with the popular
[nanoZ](http://www.white-matter.com/nanoz/) electrode impedance tester and
plating device.

### test-board-64
Test board for headstage-64. Allows injecting simulated biopotentials into
headstage modules via a selectable passive attenuator. Provides LEDs and
simulated electrical loads for optical and electrical stimulation.

### test-board-128-256
Test board for headstage, and headstage-256 modules. Allows injecting simulated
biopotentials into headstage modules via a selectable passive attenuator.
Provides LEDs and simulated electrical loads for optical and electrical
stimulation.

### [headstage-programmer](headstage-programmer/README.md)
JTAG breakout for the (Intel USB Blaster 2)[https://www.digikey.com/short/qqw7hm] used to program the headstages' MAX10 FPGA.

### pcie-analog-io [WIP]
General purpose analog IO board which sits next to pcie-host board.

## Software

### oepcie
Host libraries and language bindings for creating software applications that
acquire data from hardware in this project.

## Firmware

### kc705-host-firmware
HDL code used by the KC705 to drive the host deserializer board. _Note_ Deprecated. Will be removed upon completion of oepcie-host-firmware.

### oepcie-host-firmware
HDL code for the pcie-host board.

## Bills of materials
The bills of materials for all hardware components can be found on [this google
doc](https://docs.google.com/spreadsheets/d/1F-KWcdvH_63iXjZf0cgCfDiFX6XXW3qw6rlR8DZrFpQ/edit?usp=sharing).
Each subdirectory containing a hardware project will also have a README file
with a link to the corresponding BOM for that specific device.

## Licensing
Each subdirectory will contain a license or, possibly, a set of licenses if it involves
both hardware and software.
