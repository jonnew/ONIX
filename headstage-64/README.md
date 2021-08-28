:warning: This project has moved to a dedicated repository: https://github.com/open-ephys/onix-headstage-64
This folder should be considered archived.

# headstage-64
Serialized, multifunction headstage for small rodents. Designed to function
with [eib-64](../eib-64/README.md).

![headstage-64](./img/headstage-64.png)

## Electrophysiology
This heastage uses an [Intan RHD2164]() bioamplifier chip. This chip provides:
- 64 channels of ephys are pinned out through the large mezzanine connector on the
  bottom of the headstage
- 3 auxiliary channels
    - AUX 1 and 2 are pinned out on the bottom of the headstage to an unpopulated
      mezzanine connector
    - Channel 3 is tied to the electrical stimulator's current measurement circuit

## 3D Position Tracking
Headstage-64 featurs 4 TS4231 Vive lighthouse receivers for 3D position
tracking. They work with V2 lighthouses. To set up tracking,

1. Serial into each of the basestations using the USB connection on the back
   and set up a terminal conneciton using `screen /dev/ttyACM0 115200` or
   similar

2. Once connected you can hit Tab to see commands

3. Set the mode of one base staiton to 1 (mode 1) and the other to 2 (mode 2).

## 3D Orientation Tracking
TODO

## Neural Stimulators
Headstage-64 provides onboard electrical and optical stimulation. Stimulus
trains can be parameterized in a similar way to the master-8 or pulse pal.
Electical and optical stimulus trains cannot be delivered simultaneously.
If there is a conflict, electical stimuluation will take priority and optical
stimulus triggers will be ignored.

To acheive the shortest latency, electrical and optical stimulation can be triggered
using the GPIO1 serializer output. Because both stimulators share this trigger line,
it is important to only enable one of the devices (using its ENABLE register) prioir
to toggling this pin.

### Optical Stimulation

Q. Can i parallel the Cathode connections to increase max current
A. Yes.

### Electical Stimulation
The electrical stimulation cicuit is an improved Howland current pump followed by
an precision current measurement circuit. The current pump is supplied by +/-15V
rails and can supply up to +/- 2.5 mA. The output c

ISTIM = (VDAC  - 2.5)/1000.
e.g.
VDAC = 2.5   -> ISTIM = 0
VDAC = 5.0   -> ISTIM = 2.5 mA
VDAC = 0.0   -> ISTIM = -2.5mA

and

Imeas = 400 * ISTIM + 1.25V
e.g.
ISTIM = 0      -> IMEAS = 1.25V
ISTIM = 2.5mA  -> IMEAS = 2.25V
ISTIM = -2.5mA -> IMEAS = 0.25V

## Schematic
![headstage-64 Schematic](./img/headstage-64_schematic.png)

## Gerber Files
{% include gerber_layers.md %}

![headstage-64 Gerbers](./img/headstage-64_gerbers.png)

## Bill of Materials
The BOM is located on [this google
sheet](https://docs.google.com/spreadsheets/d/1F-KWcdvH_63iXjZf0cgCfDiFX6XXW3qw6rlR8DZrFpQ/edit#gid=138167638).

## FPGA Pinout
The FPGA pinout is located on [this google
sheet](https://docs.google.com/spreadsheets/d/1oJoQ89dJNL9LIiTrRnwJ_9KGiLzJ53Tju5Lfchuvsb0/edit#gid=2100166621).

## Connector Pinout
The headstage connector pinout (ADC input mapping, stimulation connections,
etc) is located on [this google
sheet](https://docs.google.com/spreadsheets/d/11wRDYOqHN5lPb03yUdfXfK0zvaDYsVetplaNK-R90Gg/edit#gid=663991061)
