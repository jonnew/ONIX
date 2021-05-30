# ONIX headstage-neuropix1

:warning: This headstage has moved to a dedicated repository: https://github.com/open-ephys/onix-headstage-neuropix1.
This folder should be considered archived.

Serialized, multifunction headstage targeting the neuropixels 1.0 probe. This
headstage supports serialized acquisition from:

- Two neuropixels 1.0 probes
- A BNO055 9-axis IMU for real-time, 3D orientation tracking
- Three TS4231 light to digital converters for real-time, 3D position tracking
  with HTC Vive base stations
- A high performance MAX10 FPGA for real-time processing and probe data
  correction (offset removal and gain correction)

There are two versions of this headstage in this repository, one that uses the
standard IMEC-specified ZIF connector and a second that uses a larger, more
reliable one.
