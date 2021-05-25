# Makefile for icarus verilog

# Automatically glob all .v files
SOURCES = $(wildcard *.v)

# By default, compile, simulate, and open waveform viewer
default: display

.PHONY: compile
compile: out.vvp

.PHONY: simulate
simulate: out.vcd

# View waveforms in GTKWave
.PHONY: display
display: out.vcd
	@touch out.gtkw
	gtkwave -A -f out.vcd

# Compile verilog to VVP file
out.vvp: $(SOURCES)
	iverilog -o out.vvp $(SOURCES)

# Run the simulation
out.vcd: out.vvp
	vvp out.vvp

# Delete temporary files
clean:
	rm -f out.vvp
	rm -f out.vcd
	rm -f out.gtkw

