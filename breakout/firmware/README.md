# Breakout Board Gateware
The breakout board contains a [TinyFPGA BX](https://tinyfpga.com/bx/guide.html)
(Lattice ICE40 breakout board) for digital input serialization, digital output
deserialization, interpreting user input, and driving indication LEDs.

## Build Steps
The build process is governed by the the included Makefile. There are a few key
targets:

1. `make`: makes the bit file
1. `make sim`: Perform RTL-level simuatation using iverilog
1. `make postsim`: Perform RTL-level simuatation using iverilog
1. `make prog`: makes the bit file and attempts to programm the FPGA using `tinyprog`
1. `make test`: makes the testbenches
1. `make view-test`: makes the testbenches if required and attempts to view resulting waveforms using GTKWave
1. `make clean`: cleans all built artifacts

To build and program the bit file, you will need the
[`yosys`](http://www.clifford.at/yosys/),
[`nextpnr`](https://github.com/YosysHQ/nextpnr), and
[`tinyprog`](https://pypi.org/project/tinyprog/). To build, simulate, and view
the testbenches you will need `yosys`,
[`iverilog`](http://iverilog.icarus.com/), and
[`gtkwave`](http://gtkwave.sourceforge.net/).

### Installing the Toolchain
Install `yosys`:

```
sudo apt install yosys
```

Next, you will need to build and install `nextpnr` as described
[here](https://github.com/YosysHQ/nextpnr#nextpnr-ice40).

### Installing the Programmer
Install `APIO` and `tinyprog`, open up a terminal and run the following
commands:

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

Now you can program the FPGA using

```
make prog
```

This will create all required bit files etc before programming

### Building and Examining the Testbenches
Simulation is done with Icrarus Verilog

```
sudo apt install iverilog
sudo apt install gtkwave
```

Then simply `make sim`. You can look at the resulting `.vcd` files using
gtkwave using `make view-test`.
