`timescale 1 ns / 100 ps

`include "clk_div.v"

module neopix_controller_tb();

    parameter CLK_RATE_HZ   = 50_000_000;

    // Clock and reset
    reg            i_clk;
    reg            i_reset;

    // Derived from host to breakout slow word
    reg            i_acq_running;
    reg            i_acq_reset_done;
    reg    [1:0]   i_reserved;
    reg    [3:0]   i_ledlevel;
    reg    [1:0]   i_ledmode;
    reg    [1:0]   i_porta_status;
    reg    [1:0]   i_portb_status;
    reg    [1:0]   i_portc_status;
    reg    [1:0]   i_portd_status;
    reg    [11:0]  i_aio_dir;
    reg    [1:0]   i_harp_conf;
    reg    [15:0]  i_gpio_dir;

    // Link power
    reg    [3:0]   i_link_pow;

    // HARP heartbeat
    reg            i_harp_hb;

    // Button press state
    reg    [5:0]   i_button;

    // Digital IO
    reg    [7:0]   i_din_state;
    reg    [7:0]   i_dout_state;

    // Neopixel control signal
    wire            o_neopix;


    // 50 MHz Clk
    always
    #10 i_clk = ~i_clk;

    neopix_controller # (
        .CLK_RATE_HZ(50_000_000)
    ) uut (.*);

    // Simulus
    initial begin
        $dumpfile("neopix_controller_tb.vcd");
        $dumpvars;

        i_clk = 0;
        i_reset = 1;

        #100  // Lower reset
        i_ledlevel = 7'b0000_0001;
        i_reset = 0;

        //#1000000 $finish;
        #1000000 $finish;
    end
endmodule
