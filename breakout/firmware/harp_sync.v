//`include "uart_tx.v"
`include "harp_counter.v"

module harp_sync # (
    parameter CLK_RATE_HZ = 1000000
) (
    input clk,
    input reset,
    input run,
    output TX,
    output LED
);

wire [7:0] uart_data;
wire uart_start;
wire uart_finish;
wire UART_TX;

wire uart_blank;

assign TX = UART_TX | uart_blank; //blank state for an uart is 1, so if blank is enable, or'ing will fix it at 1

uart_tx # (
    .CLK_RATE_HZ(CLK_RATE_HZ)
) tx (
    .clk(clk),
    .reset(reset),
    .data(uart_data),
    .start(uart_start),
    .finish(uart_finish),
    .UART_TX(UART_TX)
);

harp_counter # (
    .CLK_RATE_HZ(CLK_RATE_HZ)
) sync (
    .reset(reset),
    .clk(clk),
    .run(run),
    .uart_data(uart_data),
    .uart_start(uart_start),
    .uart_blank(uart_blank),
    .uart_end(uart_finish),
    .LED(LED)
);
endmodule
