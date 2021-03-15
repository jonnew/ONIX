//`include "uart_tx.v"

module uart_debugger # (
    parameter DATA_BYTES = 1,
    parameter CLK_RATE_HZ = 50_000_000,
    parameter DEAD_CLKS = 500
) (
    input wire                          i_clk,
    input wire                          i_reset,

    input wire                          i_data_valid,
    input wire[DATA_BYTES * 8 - 1:0]    i_data,

    output wire                         o_uart_tx
);

localparam DATA_BITS = DATA_BYTES * 8;

// UART transmitter
uart_tx # (
    .CLK_RATE_HZ(CLK_RATE_HZ)
) tx (
    .clk(i_clk),
    .reset(i_reset),
    .data(data_byte),
    .start(data_byte_valid),
    .finish(data_byte_sent),
    .UART_TX(o_uart_tx)
);

// Intermediate state
reg [DATA_BITS - 1:0] data_reg = 0;
reg [7:0] data_byte = 0;
reg data_byte_valid = 0;
wire data_byte_sent;

// States
localparam IDLE = 2'b00,
           SEND = 2'b01,
           WAIT = 2'b10,
           DEAD = 2'b11;

reg [1:0] state = IDLE;
reg [$clog2(DATA_BYTES):0] byte_cnt = 0;
reg [$clog2(DEAD_CLKS):0] dead_cnt = 0;

always @ (posedge i_clk) begin

    if (i_reset) begin
        byte_cnt <= 0;
        state <= IDLE;
        dead_cnt <= 0;
    end else begin

        case (state)
            IDLE: begin
                dead_cnt <= 0;
                if (i_data_valid) begin
                    data_reg <= i_data;
                    byte_cnt <= 0;
                    state <= SEND;
                end
            end
            SEND: begin
                data_byte <= data_reg[DATA_BITS - 1 -: 8];
                data_byte_valid <= 1;
                byte_cnt <= byte_cnt + 1;
                state <= WAIT;
            end
            WAIT: begin
                data_byte_valid <= 0;
                if (data_byte_sent) begin
                    if (byte_cnt == DATA_BYTES) begin
                        state <= DEAD;
                    end else begin
                        state <= SEND;
                        if (DATA_BYTES > 1) begin
                            data_reg <= {data_reg[0 +: DATA_BITS - 8],
                                         data_reg[DATA_BITS - 1 -: 8]};
                        end
                    end
                end
            end
            DEAD: begin
                dead_cnt <= dead_cnt + 1;
                if (dead_cnt == DEAD_CLKS) begin
                    state <= IDLE;
                end
            end
        endcase
    end
end

endmodule
