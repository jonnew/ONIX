# analog IO card

This board roughly conforms the PCIe card mechanical standard and uses a flex
FMC cable to connect to the master FPGA module. This board performs all
communication over FMC -- the PCIe slot design is for mechanical support and to
provide a common and standardized mounting solution across acquisition PCs

### Features

- 16 chs ADC
- 16 chs DAC
- Digital io

### Pinout

Conform to standard NI high density VHDCI pinouts to enable the use of existing cables and breakout boxes

### BOM


#### ADC

AD7616BSTZ-ND

http://www.ti.com/lit/ds/symlink/ads7950.pdf

http://www.analog.com/en/products/analog-to-digital-converters/precision-adc-20msps/simultaneous-sampling-ad-converters/ad7617.html?doc=AD7617.pdf

#### DAC

http://www.analog.com/en/products/digital-to-analog-converters/precision-dac/bipolar-da-converters/ad5767.html

http://www.analog.com/en/products/digital-to-analog-converters/precision-dac/bipolar-da-converters/ad5767.html

LTC2668CUJ-12#PBF-ND

296-25558-1-ND

#### Isolation
I dont think this is needed

#### LVDS to single ended

#### Connector

We can use Molex dual/stacked Ultra+(tm) VHDCI plugs
http://www.molex.com/molex/products/family?key=ultra__vhdci&channel=products&chanName=family&pageTitle=Introduction&parentKey=shielded_input_output_io

We likely have enough space to fit two of these on there (~44mm width each) 

## Manufacturing requirements

TODO:
- PCB thickness:
- 4 layers
- 8mil trace width & spacing
- 12mil min. drill

PCI Brackets:
http://www.mouser.com/Search/ProductDetail.aspx?R=9203virtualkey53400000virtualkey534-9203
http://www.cbttechnology.com/products/s/PCI/PCI-blanks-database.php
http://www.keyelco.com/category.cfm/Standard-Blanks/Computer-Bracket-Blanks-With-Tabs/p/427/id/704/c_id/899

There is one 0.05" pitch BGA connector (the samtech FMC connector) and a few omnetics connectors with inacessible pins on the board, so a stencil and reflow soldering need to be used.
