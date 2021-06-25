:warning: This headstage has moved to a dedicated repository: https://github.com/open-ephys/onix-fmc-host.
This folder should be considered archived.

# FMC Host Board
This card serves as host interface for serialized headstages and miniscopes, as
well as general purpose analog and digital IO. This board is a VITA-57.1
compliant module that uses high pin-count FMC connector. In combination with a
base FPGA board (e.g. [Numato Lab
Nereid](https://numato.com/product/nereid-kintex-7-pci-express-fpga-development-board),
it provides host PC communication for the following:

- Two deserailizers for any multifunction headstage conforming to our
  serialization protocol
- 12x 16-bit, +/-10V analog outputs or inputs. Direction selected via analog
  switch controllable over the FMC connector. Analog inputs are separated into
  2 simultaneously sampled 6-channel banks. When analog outputs are used, they
  are are always looped back using the analog inputs.
- 3x high speed LVDS input pairs
- 2x high speed LVDS outputs pairs
- 2x high speed, arbitrary logic-level, singled-ended lock inputs
- 1x high speed single ended, 50 ohm clock output
- 4x MLVDS input or output trigger lines

Easy access to IO is provided by the [breakout board](../breakout/README.md).

![pcie-host](./img/fmc-host.png)

## Gerber Files
{% include gerber_layers.md %}

## Bill of Materials
The bill of materials for this device can be found
[here](https://docs.google.com/spreadsheets/d/1F-KWcdvH_63iXjZf0cgCfDiFX6XXW3qw6rlR8DZrFpQ/edit#gid=1976092058).

## Manufacturing Requirements
In order to meet approximately correct trace impedances, the design assumes the following stackup:

1. Top Copper 0.035 mm
1. Prepreg (2313\*1) 0.1 mm
1. Inner Copper 0.0175 mm
1. Core (Copper) 0.565 mm
1. Inner Copper 0.0175 mm
1. Prepeg (2116\*1) 0.127 mm
1. Inner Copper 0.0175 mm
1. Core (Copper) 0.565 mm
1. Inner Copper 0.0175 mm
1. Prepreg (2313\*1) 0.1 mm
1. Bottom Copper 0.0175 mm
