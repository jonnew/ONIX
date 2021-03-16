# Breakout Board Firmware
The breakout board contains a TinyFPGA Bx (Lattice ICE40 breakout board) for
digital input serialization, digital output deserialization, interpreting user
input, and driving indication LEDs

## Build Steps
The firmware build process is governed by the the included Makefile. There are a few key targets

1. `make`: makes the firmware
2. `make prog`: makes the firmware if required and attempts to programm the FPGA using `tinyprog`
2. `make test`: makes the testbenches
2. `make view-test`: makes the testbenches if required and attempts to view resulting waveforms using GTKWave
3. `make clean`: cleans all built artifacts

To build and program the firmware, you will need the `yosys`, `nextpnr`, and
`tinyprog`. To build, simulate, and view the testbenches you will need `yosys`,
`iverilog`, and `gtkwave`. To get these dependencies do the following (these
instructions are mostly taken from https://tinyfpga.com/bx/guide.html).

### Building and Programming the Firmware
Install `APIO` and `tinyprog`, open up a terminal and run the following commands:

```
pip install apio==0.4.0b5 tinyprog
apio install system scons icestorm iverilog
apio drivers --serial-enable
```

On Unix systems, you may need to add yourself to the dialout group in order for
your user to be able to access serial ports. You can do that by running:

```
sudo usermod -a -G dialout $USER
```

Connect your TinyFPGA BX board(s) and make sure the bootloader is up to date by
running the following command:

```
tinyprog --update-bootloader
```

Now you can program the FPGA with the breakout firmware using

```
make prog
```

This will create all required bit files etc before programming


### Building and Examining the Testbenches
Simulation is done with Icrarus Verilog

```
sudo apt-get install iverilog
sudo apt-get install gtkwave
```

Then simply `make test`. You can look at the resulting `.vcd` files using
gtkwave using `make view-test`.
