# Digital Headstage System for Rats
This work is a second-order fork. It is based on the following open source
designs:

1. [Intan's headstages](http://intantech.com/index.html): no license provided
2. [Open Ephys headstages](https://github.com/open-ephys/headstage): Open
   Hardware license. Based upon (1).

__Description__ Traditionally, headstages serve as a buffer between
high-impedance electrodes recording signals from the brain and low-impedance
wires that carry signals to a rack-mounted filter bank and amplifier array.
They are sometimes referred to as "preamplifiers," because they boost current
without amplifying voltage. The components in this project make use of Intan
chips to perform both amplification and digitalization near the recording site
instead of sending analog voltages back to a computer for digitization.

This project entails a bunch of modular circuits that are aimed at rat
electrophysiology. The designs in this project acquisition from up to 256
recording electrodes, 12 auxiliary inputs (which can be user specified or
dedicated to onboard 6-axis pose sensing system), and integrated LED driver for
optogenetic stimulation (no fiber optic tether required). These designs are the
basis for rat recordings in the [Wilson Lab at MIT](http://web.mit.edu/wilsonlab/).

__Features__
- Circular form factor is aimed at conformation with **tetrode drive** assemblies
 - Low profile, stacked connectorization using 0.4mm pitch [Hirose DF40](http://www.digikey.com/product-search/en?FV=ffec4097) mezzanine headers
- SPI interface options for:
 - 128 channels using a single double data-rate LVDS bus (32 tetrodes)
 - 256 channels using two double data-rate LVDS busses (64 tetrodes)
- Integrated electrode plating and impedance testing

## Components
Each of module is described below.

#### EIB-128
128 Channel electrode interface board

#### EIB-256
256 Channel electrode interface board

#### Headstage
A very low profile 128-channel digital headstage module for amplifying, filtering, and digitizing
microwire voltage data from a microdrive implant. Up to 256 wires (64 tetrodes)
can be acquired by stacking two modules.

#### Test Board
Test board for 128 headstage module

#### Gyro Board
Gyroscope breakout board

#### SPI Interface Board
Digital interface board

#### Cable Adapter
Adapter for commutator

## Bill of materials
The bill of materials for all components can be found on [this google
doc](https://docs.google.com/spreadsheets/d/1F-KWcdvH_63iXjZf0cgCfDiFX6XXW3qw6rlR8DZrFpQ/edit?usp=sharing).

## Hardware Licensing
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img
alt="Creative Commons License" style="border-width:0"
src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br /><span
xmlns:dct="http://purl.org/dc/terms/" property="dct:title">Rat digital headstage</span> by <a xmlns:cc="http://creativecommons.org/ns#"
href="https://github.com/jonnew/cyclops" property="cc:attributionName"
rel="cc:attributionURL">Jonathan P. Newman</a> is licensed under a <a
rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative
Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.<br
/>Based on a work at <a xmlns:dct="http://purl.org/dc/terms/"
href="https://github.com/jonnew/headstage"
rel="dct:source">https://github.com/jonnew/headstage</a>.

## TODO (not up to date...)
- [ ] For the implants with VTA recordings -- where will these static
  electrodes interface with the EIB. Will they just take an couple 'tetrode
  spots', or is something more specialized needed?
    - Edit: maybe make a special EIB with a connector coming from a flex pcb
      from the VTA implants.
- [ ] In talking to Jakob, I realized that I can share a single CS, MOSI, and
  SCLK bus among all 4 intan chips assuming (1) I don't mind that all the chips
  do the same thing (2) I make sure that the accumulating decrease in the
  effective resistance of having 2X100 ohm resistors close to each other
  (multidrop config) is not going to mess with the LVDS signal integrity.
    - To implement this (in the simplest way possible) I should just be able to
      jumper the CS, MOSI, and SCLK lines at the level of SPI interface board.
      If this works without chaning any of the headstage terminations, then
      great, I just saved 6 wires!
    - Note that this strategy does not require a change to the headstages, just
      the interface board. I will move forward with this idea because it will
      allow me to use the headstages for 128 channel acquisition in the
      meantime.
- [ ] Experiments with the color based tracker indicate that a diffuser,
  preferably something very similar to a tiny ping-pong ball is required on the
  head-direction LEDS on the SPI interface. In general, the SPI interface
  should accommodate whatever solution I come up with. Also, we can get rid of
  the 'wings' on the SPI interface since the geometric tracking idea was
  ill-conceived.
    - [ ] LED choices: Use green and blue for the SPI interface board. The orange
      LED matches the room lighting and is hard to track
- [ ] I need to make sure that he DF40 headers are not going to short onto the
  vias underneath the connectors on the EIB. This should be done empirically.
  - EDIT: they do not short, but there is a different problem: one of the gold
    pins on TT23 of the EIB hits the outer edge of the DF40 receptical above
    it. This pin needs to be moved. More generally, I need to add vertical
    keepouts to the DF40 parts to prevent this in the future.

#### Completed TODOs
- [x] Analog connections terminating at the inputs to an RHD chip do not need
  to feed through to the next board. However, digital connections __always__
  should feed through because they need to make it to the headstage interface
  - Is there a way to prevent stubs in the digital feed troughs that will go
    the wrong way?
- [x] Does it make a difference if the header or the receptacle is next to the
  RHD, specifically in regard to stack height. i.e. is there going to be enough
  room for the potted chip?
  - Selected a connector that has various options for stack height, up to 3mm
    which will be more than enough
- [x] Make sure the circuit has __ample test-points__, especially for probing
  GND, VDD, REF, ELEC_TEST, AUX_OUT.
  - I did my best, but was not able to get the ELEC_TEST pinned out due to
    routing issues. However, these signals have only a standard CMOS analog
    switch between their wire entry point and the RHD chip, so I'm not sure
    test points onboard are super critical.
- [x] Make the annualar ring on test points larger. Its hard to notice how tiny
  everything on these boards are when doing the layout.
- [x] Address Reid's concern about routing high impedance ELEC_TEST lines near
  digital signals
- [x] Add three axis accelerometer to one of the RHD's aux analog inputs
  - Ask for Reid's opinion on the possibility of this chip introducing HF noise
    into the RHD chip sitting right below it.
  - Reid did not think it would be an issue
- [x] Stack height concerns __test empirically using mockups__
 - The ADX is sitting right over an RHD. This leads to an effective stack
   height of their combined thickness. Solution: only populate the to ADX since
   thats all you need anyway. 
 - Just for the record, the ADX is 1.5 mm tall. The potted RHD is 0.8 mm. Total
   stack height = 2.3 + 2.3\*0.1 ~ 2.5 mm. This is a stack height option for
   the hirose connectors, so I think even if this issue arises for some reason,
   I can simply use a taller hirose.
 - The 74HC4053 is a pretty thick chip (standard TSSOP). I got 2 mm stack
   height headers. Will it fit? Its 1.75 mm thick max, so it should fit
   (barely). Also, need to take into account that the bottom headstage is
   rotated 90 deg. relative to the top. It looks like nothing will move
   underneath the 74HC4053 so long as the bottom ADX is not populated, but I
   need to test this on some cheapo boards before making the real ones!
 - Should get stack height of potted die from Reid just to be safe.
  - EDIT: Potted chip is ~0.8 mm tall. 
- [x] Do I really need the four holes in the center of the board. They are
  meant to be fore fiber optics, but they are a serious pain when routing the
  EIBs.
  - EDIT: ended up keeping them and taking the time to route around them
- [x] The EIB needs mounting holes. Maybe something that will fit the old style
  drive bodies with a triangular mounts.
 - Idea: Use a central t-nut on the drive body. This way, there only has to be
   one hole to mount the EIB and the connection will be very strong, especially
   if some sacrificial 'bumps' are placed on the 3D printed pad that will make
   contact with the bottom of the EIB.
 - I created a standard mounting fixture called EIB_MOUNT_A. This provides a
   center hole for a M2 or 2-56 screw and two alignment holes for tabs on the
   drive body's mounting plate.
- [x] The silkscreen on the tetrodes is too small. Make it bigger and move
  toward the inner part of the EIB.
- [x] On the SPI interface, on the mock ups from OSH park, small vias failed to
  support distinctions between octagon style plated holes and round holes. This
  means that my clever way of distinguish SPI interface polarity was a bust. 
- [x] The holes for the spi wire are too small - multiply by 1.5
- [x] The boarder silkscreens on the small passives (0402 and smaller) is too
  close to the pads and is not showing up on the boards.
- [x] Add VDD and GND test points, with large vias, to the periphery of the
  SPI interface board to make for easy testing using a benchtop power supply.
  In fact, do this for all the boards that don't have it already.
- [x] The SPI interface needs to be changed in the following ways.
  - Get rid of fins, they will never work
  - Move as many LEDs to as tight of an area as possible, in between the 2nd
    and 4th LEDs on current design
  - Move the current limit resistors far away so they do not get in the way of
    the diffuser
  - Shape the PCB to fit the diffuser over the LEDs
