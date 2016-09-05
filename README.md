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

## Components
Each of module is described below.

#### EIB-128
128 Channel electrode interface board

#### EIB-256
256 Channel electrode interface board

#### Headstage
128 Channel amplifier and digitizer module. Use two for recording 256 channels.

#### Test Board
Test board for 128 headstage module

#### Gyro Board
Gyroscope breakout board

#### SPI Interface Board
Digital interface board

#### Cable Adapter
Adapter for commutator

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
rel="dct:source">https://github.com/jonnew/cyclops</a>.
