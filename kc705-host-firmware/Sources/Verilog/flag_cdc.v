`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 29.04.2016 06:36:17
// Design Name: 
// Module Name: flag_cdc
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module flag_cdc(
    input clkA,
    input clkB,
    input in,
    output out,
    output busy
    );
    
    reg toggleA = 1'b0;;
    always @(posedge clkA) toggleA <= toggleA ^ in;
    
    //three flip-flop crosser
    reg [3:0] syncA = 4'b0;
    always @(posedge clkB) syncA <= {syncA[2:0], toggleA};
    
    reg [2:0] syncB = 2'b0;
    always @(posedge clkA) syncB <= {syncB[1:0], syncA[3]};
    
    assign out = (syncA[3] ^ syncA[2]);
    assign busy = (toggleA ^ syncB[2]);
    
endmodule

module bus_cdc #( parameter WIDTH = 1)
(
    input clkDst,
    input [WIDTH-1:0] in,
    output [WIDTH-1:0] out
);

reg [WIDTH-1:0] sync [1:0];

always @(posedge clkDst)
begin
    sync[1] <= sync[0];
    sync[0] <= in;
end

assign out = sync[1];

endmodule
