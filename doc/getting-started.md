# Getting started with Open Ephys++ headstages
To use the headstages in this project in your own lab, you will need the
following components:

1. Microdrive 
1. EIB 
1. Headstage 
1. Coaxial cable\*
1. Commutator (optional)
1. FMC host board
1. Numato Nereid PCIe carrier board
1. Acquisition Software
1. Plating board (optional; 1 per lab is standard)
1. Test board 

\* Choice depends on animal size

"Starter packs" containing all the equipment you will need to get started will
soon be available though the [Open Ephys Store](http://www.open-ephys.org/store/).

Lets step through each of these and detail where to get them and how to
assemble them together to create the full system. In general, our designs focus
on chronically implanted, independently adjustable arrays of electrodes (called
microdrives). Some good starting places for learning about how to assemble
these devices are enumerated below:

1. [HD Shuttle Drive]() Details our most Our most recent drive design. It is
packed with detail on the assembly process
1. [Tetrode Fabrication with Twister3]() Our open-source tetrode twisting
   machine which greatly improves tetrode making speed and reliability compared
   to other methods.
1. [Tetrode Fabrication](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC2794082/) Dated but contains relevant info.
1. [Old-school Drive Fabriation](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC2793172/) Dated but contains relevant info.
1. TODO 

### Microdrive
The [HD-Shuttle Drives](TODO), which works with the headstages in this project, can
be purchased directly from the open ephys store. See [the paper](TODO) for assembly
instructions. 

### Electrode interface boards (EIBs)
EIBs can be purchased directly from the open ephys store. Variants are
available for both the first generation open-ephys system and the hardware in
this project. These are generally used for pinning out tetrodes. Custom EIBs
with conforming form factors can be made for a variety of probes (e.g. silicon
probes). If you want to make a new EIB for our headstages, please get in
contact using an issue on this repository se we can plan it out together.

### Headstages
[headstage-64](../headstage-64) purchased directly from the open ephys store.
It's FPGA is pre-flashed to acquire from all avaialble sensors.

### Coaxial Cable
TODO

### Commutator
TODO

### Host Acquisition Board

#### Board installation
1. Press the FMC host cared into the FMC connector on the Nereid carrier board.
1. Insert the four screws that hold the FMC host board into place on the Nereid
   carrier board (2 around FMC connector and two on front bezzel.
1. Shut down your computer and flip the switch to off on the ATX power supply.
1. Insert the Numato Nereid board/FMC host tinto an availabl PCIe slot.
1. Insert an 6-pin available ATX conenctor into the white receptible on the
   Nereid carrier board.
1. Turn your computer, on. The fan on the Nereid board should be active.
1. If your the board has not been pre-flashed, it will not be recognized when
   you reboot your computer. This is normal.

#### Programming the host board
If you have a pre-flashed and press-assembled host board, then skip to step XX.

TODO: can i program without download cable
TODO: Is there a standalone programming tool?

1. Install Vivado Webpack
1. Download the bit files here:
1. TODO: copy nereid instructions
1. Reboot your comptuer

#### Install device drivers (Windows only)
1. When your reboot, you should see unknown device in device manager (
1. TODO: copy xillybus guide
1. Reboot your computer
1. Now you should see the device show up as so:

####


### Host Software
There are currently two options for acquiring data

1. [Bonsai]()
2. [Open Ephys GUI]()

#### Using Bonsai
TODO

#### Using Open Ephys GUI
TODO
