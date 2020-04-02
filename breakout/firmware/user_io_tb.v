`timescale 1 ns / 100 ps

`include "user_io.v"
`include "./i2c-master/bench/verilog/i2c_slave_model.v"
//`include "./verilog-i2c/rtl/axis_fifo.v"
//`include "./verilog-i2c/rtl/i2c_master.v"

module user_io_tb();

    reg             clk;
    reg             reset;
    reg     [3:0]   i_led;
    wire    [7:0]   o_button;
    wire    [3:0]   o_link_pow;
    wire            io_sda;
    wire            io_scl;

    user_io # (
        .CLK_RATE_HZ(50_000_000),
        .I2C_CLK_RATE_HZ(400_000)
    ) uio (
        // Clk and reset
        .i_clk(clk),
        .i_reset(reset),
        .i_led,
        .o_button,
        .o_link_pow,
        .io_scl,
        .io_sda
    );

	// hookup i2c slave model
	i2c_slave_model # (
        .I2C_ADR(7'b010_0000) 
    ) i2c_slave (
		.scl(io_scl),
		.sda(io_sda)
	);

	pullup p1(scl); // pullup scl line
	pullup p2(sda); // pullup sda line

    // Start Test

    // 50 MHz Clk
    always
    #20 clk = ~clk;

    // Simulus
    initial begin
        $dumpfile("user_io_tb.vcd");
        $dumpvars;

        clk = 0;
        reset = 1;

        #100  // Lower reset
        reset = 0;

        //#200000 $finish;
    end

endmodule
