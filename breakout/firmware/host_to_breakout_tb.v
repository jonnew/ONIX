`timescale 1 ns / 1 ps
//`include "host_to_breakout.v"

module host_to_breakout_tb();

reg             i_clk;

reg             i_clk_s;
reg             i_d0_s;

wire            o_slow_valid;
wire [47:0]     o_slow_value;
wire            o_reset;

wire [7:0]      o_port;

host_to_breakout uut (.*);

// Hidden host clock (120 MHz)
reg             host_clk;
initial begin
    host_clk = 0;
    #100
    host_clk = 1;
    forever #4.166666666666666666 host_clk = ~host_clk;
end

// Synchronous generated clock (PLL stand-in; 60 MHz)
initial begin
    i_clk = 0;
    #100
    #2.08333333333 // Phase offset of PLL generated clock
    i_clk = 1;
    forever #8.33333333333333333 i_clk = ~i_clk;
end

// Frame clock (10 MHz)
initial begin
    i_clk_s = 0;
    #100
    i_clk_s = 1;
    forever #50 i_clk_s = ~i_clk_s;
end

// Some weird data generator
integer cnt = 0;
reg [11:0] pattern;
always @ (posedge host_clk) begin
    i_d0_s  <= pattern[11];
    pattern <= {pattern[10:0], pattern[11]};
end

// Initial blocks are sequential and start at time 0
initial begin

$dumpfile("host_to_breakout_tb.vcd");
$dumpvars();

i_d0_s = 0;

//pattern = 12'b1000_0000_0000;
pattern = 12'b0001_1100_1110; // led = 2'b01, port = 8'b11001110

#500

pattern = 12'b0101_1100_1110; // led = 2'b01, port = 8'b11001110

#2000 $finish;
end

endmodule
