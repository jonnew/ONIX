# Makefile borrowed from https://github.com/cliffordwolf/icestorm/blob/master/examples/icestick/Makefile
#
# The following license is from the icestorm project and specifically applies to this file only:
#
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted, provided that the above
#  copyright notice and this permission notice appear in all copies.
#
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

TOP = breakout
#TOP = post_assembly_test
#TOP = clk_div
#TOP = breakout_to_host
#TOP = host_to_breakout
#TOP = handshake_sync
#TOP = user_io
#TOP = uart_debugger
#TOP = input_debug
#TOP = neopix_controller

SIMFLAG =
PIN_DEF = $(TOP).pcf
DEVICE = lp8k

ALL_VER := $(wildcard *.v) $(wildcard */*.v)
SYNTH_FILES := $(filter-out $(wildcard *_tb.v), $(ALL_VER))
TB_VER := $(wildcard *_tb.v)

all: $(TOP).rpt $(TOP).bin

# Synthesis
%.json: %.v
	yosys $(SIMFLAG) -p 'synth_ice40 -top $(TOP) -json $@' $<

# PnR
%.asc: $(PIN_DEF) %.json
	nextpnr-ice40 --seed 3 --lp8k --package cm81 --json $(word 2,$^) --pcf $(word 1,$^) --asc $@ #--gui

# Write binary
%.bin: %.asc
	icepack $< $@

# Timing analysis
%.rpt: %.asc
	icetime -d $(DEVICE) -mtr $@ $<

# Write pre-synthesis simulation files
%_tb: %_tb.v %.v
	iverilog $(SIMFLAG) -o $@ $^

# Perform pre-synthesis simulation
%_tb.vcd: %_tb
	vvp -N $< +vcd=$@

# Transpile synthesized design to large .v file
%_syn.v: %.json
	yosys -p 'read_json $^; write_verilog $@'

# Write post-synthesis simulation files using primitives from $(CELLS)
%_syntb.out: %_tb.v %_syn.v
	iverilog $(SIMFLAG) -o $@ $^ `yosys-config --datdir/ice40/cells_sim.v`

# Perform post-synthesis simulation
%_syntb.vcd: %_syntb.out
	vvp -v -N $< +vcd=$@

# Pre-synthesis simulation target
.PHONY: sim
sim: SIMFLAG = -DSIMULATION
sim: $(TOP)_tb.vcd

# Post-synthesis simulation target
.PHONY: postsim
postsim: SIMFLAG = -DSIMULATION
postsim: $(TOP)_syntb.vcd

# Program the board
.PHONY: prog
prog: $(TOP).bin
	tinyprog -p $<

.PHONY: formal
formal: formal.sby
	rm -rf formal
	sby formal.sby

clean:
	rm -f $(TOP).json $(TOP).asc $(TOP).rpt $(TOP).bin *.vcd $(TOP)_syn.v $(TOP)_syntb.out

.SECONDARY:
.PHONY: all prog clean test
