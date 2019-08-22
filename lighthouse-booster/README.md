# Lighthouse Synchronization Booster
This board is a modification for [HTC Vive Base
Stations](https://www.vive.com/us/accessory/base-station/) ("lighthouses") that
increases their tracking range.

## Principle of Operation
HTC base stations work by producing alternating global synchronization flashes
using an IR LED array followed by laser curtain sweeps for 3D localization.
Because we use tiny photodiodes on our headstages (with about 1/10 the
effective area compared to those in the HTC headset), they are only capable of
working at ~3 meters from an unmodified base station. This issue is entirely
caused by the power of the global synchronization pulse. The synchronization
pulse must simultaneously blanket the entire room in IR light and therefore its
irradiance goes down as the square of distance from the base station.
Therefore, a very powerful LED array must be used. Conversely, because the
laser curtains are 2D sheets, their irradiance goes down linearly with distance
so they are easily received by the headstage at large distances without
modification.

This board is a drop in replacement for the standard LED array inside the HTC
Vive base station. It replaces the standard 9-LED array with an array of 120
LEDs.  Each set of 15 is controlled by a fast, low-side transistor switch. The
board draws about 700 mA DC at 12V. The pulse current (~60 us long, 2
MHz-modulated pulses operating at ~1% duty-cycle) is approximately 200A, with
5A for each LED. During a pulse, the LED array is decoupled from the input power
using a high-side load-switch. This means that all pulse energy is taken from local
capacitance such that the rails the driving motors inside the base station
are not transiently loaded.

## Power, Thermals, and Effective Tracking Distances
TODO

## Installation
TODO

## Gerber Files
{% include gerber_layers.md %}

## Bill of Materials
The BOM is located on [this google
sheet](https://docs.google.com/spreadsheets/d/1F-KWcdvH_63iXjZf0cgCfDiFX6XXW3qw6rlR8DZrFpQ/edit#gid=1349390549).

## License
Copyright Jonathan P. Newman 2017.

This documentation describes Open Hardware and is licensed under the
CERN OHL v.1.2.

You may redistribute and modify this documentation under the terms of the CERN
OHL v.1.2. (http://ohwr.org/cernohl). This documentation is distributed WITHOUT
ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY
QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see the CERN OHL v.1.2 for
applicable conditions
