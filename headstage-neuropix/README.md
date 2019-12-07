# headstage-neuropix
Serialized, multifunction headstage targeting the neuropixels 1.0 probe. This
headstages supports serialized acqusition from:

- Two neuropixels 1.0 probes.
- A BNO055 9-axis IMU for real-time, 3D orientation tracking.
- Two TS4231 light to digital converters for real-time, 3D position tracking
  with HTC Vive base stations
- A high performance MAX10 FPGA for real-time processing

## Schematic
![headstage-neuropix Schematic](./img/headstage-neuropix_schematic.png)

## Gerber Files
{% include gerber_layers.md %}

![headstage-neuropix Gerbers](./img/headstage-neuropix_gerbers.png)

## Bill of Materials
The BOM is located on [this google
sheet](https://docs.google.com/spreadsheets/d/1F-KWcdvH_63iXjZf0cgCfDiFX6XXW3qw6rlR8DZrFpQ/edit#gid=1284354220)

## License
Copyright Jonathan P. Newman and Jakob Voigts 2019.

This documentation describes Open Hardware and is licensed under the
CERN OHL v.1.2.

You may redistribute and modify this documentation under the terms of the CERN
OHL v.1.2. (http://ohwr.org/cernohl). This documentation is distributed WITHOUT
ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY
QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see the CERN OHL v.1.2 for
applicable conditions
