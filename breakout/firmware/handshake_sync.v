// Handshake sychronizer for a vector of register values
//
module handshake_sync # (
    parameter WIDTH = 8 
) (
    input   wire                    i_wr_clk,
    input   wire                    i_rd_clk,
    input   wire   [WIDTH - 1:0]    i_data,
    output  reg    [WIDTH - 1:0]    o_data
);

reg [WIDTH - 1:0]  x_data = 'b0;
reg data_valid = 'b0;
reg wr_req = 'b0;
reg wr_ack = 'b0;
reg x_req = 'b0;
reg x_ack = 'b0;
reg rd_req = 'b0;
reg last_rd_req = 'b0;
wire busy;

// Data transmitter (i_wr_clk)
always @(posedge i_wr_clk) begin

    if (!busy && !data_valid) begin
        x_data <= i_data;
        data_valid <= 'b1;
    end else if (!busy && data_valid) begin
        wr_req <= 'b1;
    end else if (wr_ack) begin
        data_valid <= 'b0;
        wr_req <= 'b0;
    end
end

assign busy = wr_req || wr_ack;

// Request CDC
always @(posedge i_rd_clk)
	{ last_rd_req, rd_req, x_req } <= { rd_req, x_req, wr_req };

// Ack CDC
always @(posedge i_wr_clk)
	{ wr_ack, x_ack } <= { x_ack, last_rd_req };

// Data output receiver (i_rd_clk)
always @(posedge i_rd_clk) begin

    if (rd_req && !last_rd_req) begin
        o_data <= x_data;
    end

end

endmodule
