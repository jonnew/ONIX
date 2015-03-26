<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [128 channel headstage module for rat tetrode drives](#128-channel-headstage-module-for-rat-tetrode-drives)
  - [Features](#features)
  - [Bill of materials](#bill-of-materials)
  - [TODO](#todo)
  - [Hardware and Documentation Licensing](#hardware-and-documentation-licensing)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

### 128 channel headstage module for rat tetrode drives
A very low profile digital headstage for amplifying, filtering, and digitizing microwire voltage data from a microdrive implant. Up to 256 wires (64 tetrodes) can be acquired.

#### Features
- Circular form factor is aimed at conformation with **tetrode drive** assemblies
 - Low profile, stacked connectorization using 0.4mm pitch [Hirose DF40](http://www.digikey.com/product-search/en?FV=ffec4097) mezzanine headers
- SPI interface options for:
 - 128 channels using a single double data-rate LVDS bus (32 tetrodes)
 - 256 channels using two double data-rate LVDS busses (64 tetrodes)
- Integrated electrode plating and impedance testing

#### Bill of materials
The bill of materials for the headstage, EIBs, spacer boards, and SPI interface boards can be found on [this google doc](https://docs.google.com/spreadsheets/d/1F-KWcdvH_63iXjZf0cgCfDiFX6XXW3qw6rlR8DZrFpQ/edit?usp=sharing).

#### TODO
- [x] Analog connections terminating at the inputs to an RHD chip do not need to feed through to the next board. However, digital connections __always__ should feed through because they need to make it to the headstage interface
 - Is there a way to prevent stubs in the digital feed troughs that will go the wrong way?
- [x] Does it make a difference if the header or the receptacle is next to the RHD, specifically in regard to stack height. i.e. is there going to be enough room for the potted chip?
 - Selected a connector that has various options for stack height, up to 3mm which will be more than enough
- [x] Make sure the circuit has __ample test-points__, especially for probing GND, VDD, REF, ELEC_TEST, AUX_OUT.
 - I did my best, but was not able to get the ELEC_TEST pinned out due to routing issues. However, these signals have only a standard CMOS analog switch between their wire entry point and the RHD chip, so I'm not sure test points onboard are super critical.
- [ ] Make the annualar ring on test points larger. Its hard to notice how tiny everything on these boards are when doing the layout.
- [x] Address Reid's concern about routing high impedance ELEC_TEST lines near digital signals
- [x] Add three axis accelerometer to one of the RHD's aux analog inputs
 - Ask for Reid's opinion on the possibility of this chip introducing HF noise into the RHD chip sitting right below it.
 - Reid did not think it would be an issue
- [ ] Stack height concerns __test empirically using mockups__
 - The ADX is sitting right over an RHD. This leads to an effective stack height of their combined thickness. Solution: only populate the to ADX since thats all you need anyway. 
 - Just for the record, the ADX is 1.5 mm tall. The potted RHD is 0.8 mm. Total stack height = 2.3 + 2.3*0.1 ~ 2.5 mm. This is a stack height option for the hirose connectors, so I think even if this issue arises for some reason, its fixable. 
 - The 74HC4053 is a pretty thick chip (standard TSSOP). I got 2 mm stack height headers. Will it fit? Its 1.75 mm thick max, so it should fit (barely). Also, need to take into account that the bottom headstage is rotated 90 deg. relative to the top. It looks like nothing will move underneath the 74HC4053 so long as the bottom ADX is not populated, but I need to test this on some cheapo boards before making the real ones!
 - Should get stack height of potted die from Reid just to be safe.
  - EDIT: Potted chip is ~0.8 mm tall. 
- [x] Do I really need the four holes in the center of the board. They are meant to be fore fiber optics, but they are a serious pain when routing the EIBs.
  - EDIT: ended up keeping them and taking the time to route around them
- [x] The EIB needs mounting holes. Maybe something that will fit the old style drive bodies with a triangular mounts.
 - Idea: Use a central t-nut on the drive body. This way, there only has to be one hole to mount the EIB and the connection will be very strong, especially if some sacrificial 'bumps' are placed on the 3D printed pad that will make contact with the bottom of the EIB.
 - I created a standard mounting fixture called EIB_MOUNT_A. This provides a center hole for a M2 or 2-56 screw and two alignment holes for tabs on the drive body's mounting plate.
- [ ] I need to make sure that he DF40 headers are not going to short onto the vias underneath the connectors on the EIB. This should be done empirically.
- [ ] The silkscreen on the tetrodes is too small. Make it bigger and move toward the inner part of the EIB.
- [ ] For the implants with VTA recordings -- where will these static electrodes interface with the EIB. Will they just take an couple 'tetrode spots', or is something more specialized needed?
- [ ] In talking to Jakob, I realized that I could share a single CS, MOSI, and SCLK bus among all 4 intan chips assuming (1) I don't mind that all the chips do the same thing (2) I make sure that the accumlating decrease in the effective resistance of having 2X100 ohm resistors close to eachother (multidrop config) is not going to mess with the LVDS signal integrity.
 - To impelment this (in the simplest way possible) I should just be able to jumper the CS, MOSI, and SCLK lines at the level of SPI interface board. If this works without chaning any of the headstage terminations, then great, I just saved 6 wires!
- [ ] On the SPI interface, on the mock ups from OSH park, small vias failed to support distinctions between octagon style plated holes and round holes. This means that my clever way of disiginusigh SPI interface polarity was a bust. 
- [ ] The holes for the spi wire are too small - muliply by 1.5
- [ ] The boarder silkscreens on the small passives (0402 and smaller) is too close to the pads and is not showing up on the boards.

#### Hardware and Documentation Licensing
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" property="dct:title">128 Channel Circular Hirose Headstage Module</span> by <a xmlns:cc="http://creativecommons.org/ns##" href="https://github.com/jonnew/cyclops" property="cc:attributionName" rel="cc:attributionURL">Jonathan P. Newman</a> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.<br />Based on a work at <a xmlns:dct="http://purl.org/dc/terms/" href="https://github.com/jonnew/cyclops" rel="dct:source">https://github.mit.edu/jpnewman/headstage</a>.


