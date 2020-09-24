`timescale 1 ns / 1 ps

module handshake_sync_tb();

//reg             i_reset;
reg             i_wr_clk;
reg             i_rd_clk;
reg     [9:0]   i_data;
wire    [9:0]   o_data;

handshake_sync # (
    .WIDTH(10)
) uut (
    .*
);

initial begin
    i_wr_clk = 0;
    #100
    i_wr_clk = 1;
    forever #5 i_wr_clk = ~i_wr_clk;
end

initial begin
    i_rd_clk = 1;
    forever #29.3 i_rd_clk = ~i_rd_clk;
end

reg [9:0] count = 'b0;
always @(posedge i_wr_clk) begin
    count <= count +1;
    i_data <= count;
end

// Initial blocks are sequential and start at time 0
initial begin

$dumpfile("handshake_sync_tb.vcd");
$dumpvars();

#3000 $finish;
end
endmodule
