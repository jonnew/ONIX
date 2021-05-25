`timescale 1 ns / 100 ps

module uart_debugger_tb;

    reg             i_clk;
    reg             i_reset;

    reg             i_data_valid_0;
    reg     [7:0]   i_data_0;
    wire            o_uart_0;

    reg             i_data_valid_1;
    reg     [15:0]  i_data_1;
    wire            o_uart_1;

    uart_debugger # (
        .DATA_BYTES(1),
        .CLK_RATE_HZ(50_000_000)
    ) uut0 (
        .i_clk(i_clk),
        .i_reset(i_reset),
        .i_data_valid(i_data_valid_0),
        .i_data(i_data_0),
        .o_uart_tx(o_uart_0)
    );

    uart_debugger # (
        .DATA_BYTES(2),
        .CLK_RATE_HZ(50_000_000)
    ) uut1 (
        .i_clk(i_clk),
        .i_reset(i_reset),
        .i_data_valid(i_data_valid_1),
        .i_data(i_data_1),
        .o_uart_tx(o_uart_1)
    );

    // Start Test

    // 50 MHz Clk
    always
    #10 i_clk = ~i_clk;

    // Simulus
    initial begin
        $dumpfile("uart_debugger_tb.vcd");
        $dumpvars;

        i_clk = 0;
        i_reset = 1;

        #100  // Lower reset
        i_reset = 0;

        #100
        i_data_valid_0 = 1;
        i_data_0 = 8'b11001111;
        i_data_valid_1 = 1;
        i_data_1 = 16'b0011000011110000;


        #2000000 $finish;
    end

endmodule

