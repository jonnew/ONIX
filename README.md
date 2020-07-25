[ONIX](https://jonnew.github.io/onix) is a collection
of [ONI](https://github.com/jonnew/ONI)-compatible hardware, firmware, and API
for serialized, very-high channel count, closed-loop electrophysiology. It is
an evolution of the first-generation hardware and software introduced in [Open
Ephys project](http://www.open-ephys.org/) and involves many of the same
developers.  The firmware and API are general purpose -- they can be used to
acquire from and control custom headstages with arbitrary arrangements of
sensors and actuators (e.g. cameras, bioamplifier chips, LED drivers, etc.) and
are not limited to the hardware in this repository.

__Citing this work__: 

1. Citing the paper

  - TODO

2. Citing the repository itself

  - [![DOI](https://zenodo.org/badge/95248663.svg)](https://zenodo.org/badge/latestdoi/95248663)

## Features
- Follows the ONI-specification for serialization protocols, host communication
  protocols, device drivers, and host API
- Firmware and API permit acquisition and control of arbitrary arrangements of
  sensors and actuators:

    - Headstages
    - Miniscopes
    - Photometry systems
    - Etc.

- Submillisecond round-trip communication from brain, through host PC's main
  memory, and back again.
- Headstages:

    - 64-, 128-, 256-channels of electrophysiology
    - Optogenetic stimulation
    - Electrical stimulation
    - 3D-pose measurement
    - Data, user control, and power via a tiny coaxial cable
    - Wireless communication

- Low-level API implementation
- High-level API language bindings and existing integration with [Open Ephys
  GUI](http://www.open-ephys.org/gui/) and [Bonsai](http://bonsai-rx.org/).
- Quality documentation and easy routes to purchasing assembled devices.

## ONI-Compatible
The [Open Neuro Interface](https://github.com/jonnew/ONI) formally specifies data
serialization, host/PC communication, firmware blocks, device drivers and
programming interfaces for this project. All firmware, software, and hardware
artifacts in this repository are implementations of this specification.
Therefore, third party implementations that maintain compatibility with ONI
will interoperate with the software and hardware within this project.

## Software

### [Programming Interface](api/README.md)
High-performance, host-side programming interfaces for integration with
existing software and the creation of high level language bindings. 

### [Bonsai Package]()
- TODO: Myget distribution

### [Open Ephys Plugin]()
- TODO: Integration into open-ephys master

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

### [fmc-host](fmc-host/README.md)
Base board for facilitating PCIe communication, via FMC compatiable and
PCIe-capable FPGA based board (e.g. [Numato Lab
Nereid](https://numato.com/product/nereid-kintex-7-pci-express-fpga-development-board).
This board plugs into the FMC connector on the base board. It provides
communication with one headstage and lots of other analog and digital IO.

### [analog-io-breakout](analog-io-breakout/README.md)
Passive breakout board for acquiring and generating analog signals through BNC,
SMA, ribbon, or straight wire connections. Plugs into fmc-host using a 26-pin
shrunk delta ribbon cable.

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
