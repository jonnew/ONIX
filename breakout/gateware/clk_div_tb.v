`timescale 1 ns / 100 ps
//`include "clk_div.v"

module clk_div_tb();

    reg             i_reset;
    reg             i_clk;
    wire            o_clk2;
    wire            o_clk3;
    wire            o_clk4;
    wire            o_clk5;
    wire            o_clk6;
    wire            o_clk6p;

    clk_div #(.N(2)) uut2 (.*, .o_clk(o_clk2));
    clk_div #(.N(3)) uut3 (.*, .o_clk(o_clk3));
    clk_div #(.N(4)) uut4 (.*, .o_clk(o_clk4));
    clk_div #(.N(5)) uut5 (.*, .o_clk(o_clk5));
    clk_div #(.N(6)) uut6 (.*, .o_clk(o_clk6));
    clk_div #(.N(6), .PULSE(1)) uut6p (.*, .o_clk(o_clk6p));

    // Create clock
    always
    #5 i_clk = ~i_clk; // every 5 nanoseconds invert

    // Initial blocks are sequential and start at time 0
    initial begin
    $dumpfile("clk_div_tb.vcd");
    $dumpvars();

    i_reset = 0;
    i_clk = 0;

    #300 $finish;
    end

endmodule
