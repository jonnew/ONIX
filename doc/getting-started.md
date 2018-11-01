# Getting started with Open Ephys++ headstages
To use the headstages in this project in your own lab, you will need the
following components:

1. Microdrive 
1. EIB 
1. Headstage 
1. Coaxial cable\*
1. Commutator (optional)
1. Host board\*\*
1. FMC cable\*\*
1. KC705\*\*
1. Acquisition Software
1. Plating board (optional; lab shared)
1. Test board (optional, but not really)

\* Choice depends on animal size
\*\* Will likely be combined in future

"Starter packs" containing all the equipment you will need to get started will
soon be available though the [Open Ephys Store](http://www.open-ephys.org/store/).

Lets step through each of these and detail where to get them and how to
assemble them together to create the full system. In general, our designs focus
on chronically implanted, independently adjustable arrays of electrodes (called
microdrives). Some good starting places for learning about how to assemble
these devices are enumerated below:

1. [HD Shuttle Drive]() Details our most Our most recent drive design. It is
packed with detail on the assembly process
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
probes). Pin out

### Headstages
[headstage-64](../headstage-64) purchased directly from the open ephys store.
It's FPGA is pre-flashed to acquire from all avaialble sensors.

### Coaxial Cable
TODO

### Commutator
TODO

### Host Acquisition Board
TODO

### Host Software
There are currently two options for acquiring data

1. [Bonsai]()
2. [Open Ephys GUI]()

#### Using Bonsai
TODO

#### Using Open Ephys GUI
TODO
