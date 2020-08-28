# headstage-256
Serialized, multifunction headstage for large rodents. Supports both 128 or 256
channels. Designed to interface with [eib-128](../eib-128/README.md) or
[eib-256](../eib-256/README.md)

![headstage-256](./img/headstage-256.png)

## Schematic 
![headstage-256 Schematic](./img/headstage-256_schematic.png)

## Gerber Files
{% include gerber_layers.md %}

![headstage-256 Gerbers](./img/headstage-256_gerbers.png)

## Bill of Materials
The BOM is located on [this google
sheet](https://docs.google.com/spreadsheets/d/1F-KWcdvH_63iXjZf0cgCfDiFX6XXW3qw6rlR8DZrFpQ/edit#gid=1075887549)

## FPGA Pinout
The FPGA pinout is located on [this google
sheet](https://docs.google.com/spreadsheets/d/1oJoQ89dJNL9LIiTrRnwJ_9KGiLzJ53Tju5Lfchuvsb0/edit#gid=1588805600)

## Connector Pinout
The headstage connector pinouts (ADC input mapping, stimulation connections,
etc) ares are located on these google sheets:

- [eib-128 channel mapping](https://docs.google.com/spreadsheets/d/11wRDYOqHN5lPb03yUdfXfK0zvaDYsVetplaNK-R90Gg/edit#gid=663991061)
- [eib-256 channel mapping](https://docs.google.com/spreadsheets/d/11wRDYOqHN5lPb03yUdfXfK0zvaDYsVetplaNK-R90Gg/edit#gid=538743909)

## Expansion Boads
headstage-256 has two exposed headers that supply power and general purpose analog or digital IO to expand 
its functionality. Subdirectories with the prefix 'expansion` are boards that fit into these headers to 
supply various auxiliary functions. These include:

1. expansion-uphone: ultrasonic micrphone expansion board
2. expansion-long-range: large active area light house receiver board to expand 3D tracking range by several meters.
3. expansion-debug: pinout GPIO to large 0.1" pitch headers.

