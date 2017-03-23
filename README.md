# A set of hardware for serialized, very-high channel count electrophysiology (recording and stimulation)

__Maintainer__ jonnew 

__Note__This work is a second-order fork. It is based on the following open source
designs:

1. [Intan's headstages](http://intantech.com/index.html): no license provided
2. [Open Ephys headstages](https://github.com/open-ephys/headstage): Open
   Hardware license. Based upon (1).

__Description__ Traditionally, headstages serve as a buffer between
high-impedance electrodes recording signals from the brain and low-impedance
wires that carry signals to a rack-mounted filter bank and amplifier array.
They are sometimes referred to as "preamplifiers," because they serve to reduce
the source impedance of recording electrodes by increasing current drive
without providing voltage amplification. In this "one physical connection per
signal" scenario, auxiliary sensors (e.g. inertial measurement units, indication
LEDs, etc) and brain stimulator devices (e.g. electrical microstimulation or
optical inputs) each mandate an additional electrical or optical connection to
the animal subject, increasing tether weight and tendency to failure. 

The goal of this project is to take advantage of commodity integrated circuits
and FPGAs in order to reduce physical connectorization to a single, miniature
coaxial cable while maintaining extremely high channel counts (up to 1000).
Further, we sought to include sufficient onboard computation power in
orchestrate data acquisition and generate stimulation patterns. To this end, we
present a set of free and open-source modular circuits designs allowing very
high channel count eletrophysiology and optogenetics in freely moving rats
tethered bu a single miniature coaxial cable. Further, we provide host-side
hardware and firmware required to seamlessly integrate our headstage system
with several widely used free and open-source data equations platforms (The
Open Ephys GUI and Bonsai). Unlike previous designs, our device makes use of
the PCIe bus to transfer data and therefore achieves submillisecond round-trip
latencies to host-PC main memory.  Finally, we provide example recordings and
designs for a fully integrated system in the form of 60-tetrode/4-optetrode
microdrive array targeting recordings in hippocampus and optical recording and
stimulation of freely moving TH-Cre rats.

__Major Features__

- Integrated electrophysiology (up to 1000 channels), optogenetic or electrical
  micro stimulation (i.e. no fiber optic tether), and 9 DOF pose measurment.
- Submillisecond round-trip communication with host PC's main memory
- Data and commands serialized over a single coaxial cable (i.e. no 
- Modular design allows custom integration of individual project compnents.
- Integrated electrode plating and impedance testing
- Quality documentation and easy routes to purchased assembled devices.
- _Application example_: battle-tested, 64-tetrode optogenetics-capable
  microdrive that froms the basis for rat recordings in the [Wilson Lab at
  MIT](http://web.mit.edu/wilsonlab/).

- Circular form factor is 
 - Low profile, stacked connectorization using 0.4mm pitch [Hirose DF40](http://www.digikey.com/product-search/en?FV=ffec4097) mezzanine headers
- SPI interface options for:
 - 128 channels using a single double data-rate LVDS bus (32 tetrodes)
 - 256 channels using two double data-rate LVDS busses (64 tetrodes)

that are aimed at rat
electrophysiology. The designs in this project acquisition from up to 256
recording electrodes, 12 auxiliary inputs (which can be user specified or
dedicated to onboard 6-axis pose sensing system), and integrated LED driver for
optogenetic stimulation (no fiber optic tether required). These designs are the

## Components
The modules of this repository, each corresponding to a top-level directory of the same name, is described below.

#### EIB-128
128 Channel electrode interface board

#### EIB-256
256 Channel electrode interface board

#### Headstage-128
A low profile 128-channel digital headstage module for amplifying, filtering, and digitizing
microelectrode voltage data from a microdrive implant. Up to 256 wires (64 tetrodes)
can be acquired by stacking two modules.

#### Headstage-256
TODO

#### Test Board
Test board for headstage modules.

#### SERDES Interface Board
Data serailization board.

#### Cable Adapter
Adapter for commutator

#### Base FMC module
VITA-57 Complient base board for facilitating PCIe communication with host computer.

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
- [ ] I need to make sure that he DF40 headers are not going to short onto the
  vias underneath the connectors on the EIB. This should be done empirically.
  - EDIT: they do not short, but there is a different problem: one of the gold
    pins on TT23 of the EIB hits the outer edge of the DF40 receptical above
    it. This pin needs to be moved. More generally, I need to add vertical
    keepouts to the DF40 parts to prevent this in the future.
