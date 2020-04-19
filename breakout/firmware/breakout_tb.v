`timescale 1 ns / 100 ps
`include "breakout.v"

module breakout_tb();

    reg             CLK;
    reg     [7:0]   D_IN;
    reg     [1:0]   LVDS_IN;

    wire    [7:0]   D_OUT;
    wire    [2:0]   LVDS_OUT;

    wire            I2C_SCL;
    wire            I2C_SDA;

    wire            HARP_CLK_OUT;
    wire            LED;
    wire            USBPU;

    breakout uut (.*);

    // 16 MHz Clk
    always
    #62.5 CLK = ~CLK;

    initial begin
        $dumpfile("top_tb.vcd");
        $dumpvars;

        CLK = 0;
        D_IN = 0;
        LVDS_IN = 0;

        #100
        D_IN = 8'b0000001;

        #2000000 $finish;

    end
endmodule
