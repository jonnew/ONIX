`timescale 1 ns / 100 ps

`include "./i2c-master/bench/verilog/i2c_slave_model.v"
//`include "./verilog-i2c/rtl/axis_fifo.v"
//`include "./verilog-i2c/rtl/i2c_master.v"

module user_io_tb;

    reg             i_clk;
    reg             i_reset;
    reg     [1:0]   i_porta_status;
    reg     [1:0]   i_portb_status;
    reg     [1:0]   i_portc_status;
    reg     [1:0]   i_portd_status;
    wire    [7:0]   o_button;
    wire    [3:0]   o_link_pow;
    wire            io_sda;
    wire            io_scl;

    user_io # (
        .CLK_RATE_HZ(50_000_000),
        .I2C_CLK_RATE_HZ(400_000)
    ) uio (.*);

	// Hookup I2C slave model
	i2c_slave_model # (
        .I2C_ADR(7'b010_0000) 
    ) i2c_slave (
		.scl(io_scl),
		.sda(io_sda)
	);

	pullup p1(io_scl); // pullup scl line
	pullup p2(io_sda); // pullup sda line

    // Start Test

    // 50 MHz Clk
    always
    #20 i_clk = ~i_clk;

    // Simulus
    initial begin
        $dumpfile("user_io_tb.vcd");
        $dumpvars;

        i_clk = 0;
        i_reset = 1;

        #100  // Lower reset
        i_reset = 0;

        //#200000 $finish;
    end

endmodule
