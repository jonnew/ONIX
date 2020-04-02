`timescale 1 ns / 100 ps
`include "breakout_to_host.v"

module breakout_to_host_tb();

    reg         i_reset;
    reg         i_clk;
    reg [7:0]   i_port;
    reg [7:0]   i_button;
    reg [3:0]   i_link_pow;

    wire        o_clk;
    wire        o_q0;
    wire        o_q1;

    breakout_to_host uut (.*);

    // 200 MHz clock
    always
    #5 i_clk = ~i_clk;

    // Initial blocks are sequential and start at time 0
    initial begin

    //$dumpfile("breakout_input_tb.vcd");
    //$dumpvars(0, breakout_input_tb);

    // at time 0
    //$display($time, "Sim begin");
    i_clk = 1;
    i_reset = 1;
    i_port = 0;
    i_button = 0;
    i_link_pow = 0;

    // at time 100 ns, lower reset
    #100  i_reset = 0;

    i_port = 8'b11110000;
    i_button = 8'b10101010;
    i_link_pow = 4'b1000;

    #200  i_reset = 1;
    #300  i_reset = 0;
    #500  i_reset = 0;

    i_port = 8'b11110000;
    i_button = 8'b10101010;
    i_link_pow = 4'b1111;

    //#1500 $finish;
    end

endmodule
