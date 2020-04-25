`timescale 1 ns / 100 ps
//`include "breakout_to_host.v"

module breakout_to_host_tb();

reg         i_clk;
reg         i_reset;

reg [7:0]   i_port;
reg [7:0]   i_button;
reg [3:0]   i_link_pow;

wire        o_clk_s;
wire        o_d0_s;
wire        o_d1_s;

breakout_to_host uut (.*);

// 50 MHz clock
always
#10 i_clk = ~i_clk;

// Initial blocks are sequential and start at time 0
initial begin

$dumpfile("breakout_to_host_tb.vcd");
$dumpvars();

i_clk = 1;
i_port = 0;
i_button = 0;
i_link_pow = 0;
i_reset = 1;

#100
i_reset = 0;

#500 

i_port = 8'b11110000;
i_button = 8'b10101010;
i_link_pow = 4'b1000;

#500 

i_port = 8'b11110000;
i_button = 8'b10101010;
i_link_pow = 4'b1111;

#1500 $finish;
end

endmodule
