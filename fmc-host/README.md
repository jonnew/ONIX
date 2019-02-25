# FMC Host Board
This card serves as host interface for serialized headstages, as well as
general purpose analog and digital IO. This board is a VITA-57.1 compliant
module that uses low pin-count FMC connector. In combination with a base FPGA
board (e.g. [Numato Lab
Nereid](https://numato.com/product/nereid-kintex-7-pci-express-fpga-development-board),
it provides host PC communication for the following:

- One deserailizer for any multifunction headstage conforming to our
  serialization protocol
- 16x 14-bit, +/-10V analog inputs separated into 2 simultaneously sampled
  8-channel banks
- 8x 16-bit, +/-10V analog outputs
- 2x high speed, arbitrary logic-level, clock inputs
- 1x high speed clock output

Easy access to analog IO is provided by the [analog-io-breakout
board](../analog-io-breakout/README.md).

![pcie-host](./img/fmc-host.png)

## Gerber Files
{% include gerber_layers.md %}

## Bill of Materials
The bill of materials for this device can be found
[here](https://docs.google.com/spreadsheets/d/18WfmbLGt8bGUUdksKp6AKA_wMX2SJ3Tndin-nnEgUCs/edit?usp=sharing).

## Manufacturing Requirements
In order to meet approximately correct trace impedances, the design further is
assuming the following stackup:

![stackup](./img/stackup.png)

### Pinout
FMC LPC pinout can be found
[here](https://docs.google.com/spreadsheets/d/18WfmbLGt8bGUUdksKp6AKA_wMX2SJ3Tndin-nnEgUCs/edit#gid=584734392)
