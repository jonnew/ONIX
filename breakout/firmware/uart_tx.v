module uart_tx # (
    parameter CLK_RATE_HZ = 1000000,
    parameter COUNTER_WIDTH = $clog2(CLK_RATE_HZ / 100000)
) (
    input clk,
    input reset,
    input [7:0] data,
    input start,
    output reg finish,
    
    output reg UART_TX
);
    
reg [7:0] data_reg;

localparam CYCLES_PER_BIT = CLK_RATE_HZ / 100000;

reg [COUNTER_WIDTH - 1 : 0] counter = 'b0;

localparam s_idle = 2'd0,
          s_start = 2'd1,
          s_bit = 2'd2,
          s_stop = 2'd3;

reg [1:0] state = s_idle;
reg [3:0] bit = 'b0;

always @(posedge clk or posedge reset)
begin
    if (reset) begin
        counter <= 'b0;
        bit <= 'b0;
        finish <= 1'b0;
        UART_TX <= 1'b1;
        state <= s_idle;
    end else begin
        finish <= 1'b0;
        counter <= 'b0;
        case (state)
            s_idle: begin
                bit <= 'b0;
                if (start) begin
                    state <= s_start;
                    data_reg <= data;
                end 
            end
            s_start: begin
                UART_TX <= 1'b0;
                if (counter < CYCLES_PER_BIT - 1) counter <= counter + 1'b1;
                else state <= s_bit;
            end
            s_bit: begin
                UART_TX <= data_reg[bit];
                if (counter < CYCLES_PER_BIT - 1) counter <= counter + 1'b1;
                else begin
                    if (bit < 7) bit <= bit + 1'b1;
                    else state <= s_stop;
                end
            end
            s_stop: begin
                UART_TX <= 1'b1;
                if (counter < CYCLES_PER_BIT - 1) counter <= counter + 1'b1;
                else begin
                    state <= s_idle;
                    finish <= 1'b1;
                end
            end
        endcase
    end
end
    
endmodule
