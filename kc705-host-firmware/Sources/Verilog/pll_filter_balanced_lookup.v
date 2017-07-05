`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 28.04.2016 00:46:21
// Design Name: 
// Module Name: filter_balanced_lookup
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


module pll_filter_balanced_lookup(
    input clk,
    input [6:0] divider,
    output reg [9:0] value
    );
    (*rom_style = "block" *) reg [9:0] lookup [0:64];
    wire [5:0] addr;
   initial
   begin
           lookup[0]=10'b0011_0111_00;
           lookup[1]=10'b0011_0111_00;
           lookup[2]=10'b0101_1111_00;
           lookup[3]=10'b0111_1111_00;
           lookup[4]=10'b0111_1011_00;
           lookup[5]=10'b1101_0111_00;
           lookup[6]=10'b1110_1011_00;
           lookup[7]=10'b1110_1101_00;
           lookup[8]=10'b1111_1101_00;
           lookup[9]=10'b1111_0111_00;
           lookup[10]=10'b1111_1011_00;
           lookup[11]=10'b1111_1101_00;
           lookup[12]=10'b1111_0011_00;
           lookup[13]=10'b1110_0101_00;
           lookup[14]=10'b1111_0101_00;
           lookup[15]=10'b1111_0101_00;
           lookup[16]=10'b1111_0101_00;
           lookup[17]=10'b1111_0101_00;
           lookup[18]=10'b0111_0110_00;
           lookup[19]=10'b0111_0110_00;
           lookup[20]=10'b0111_0110_00;
           lookup[21]=10'b0111_0110_00;
           lookup[22]=10'b0101_1100_00;
           lookup[23]=10'b0101_1100_00;
           lookup[24]=10'b0101_1100_00;
           lookup[25]=10'b1100_0001_00;
           lookup[26]=10'b1100_0001_00;
           lookup[27]=10'b1100_0001_00;
           lookup[28]=10'b1100_0001_00;
           lookup[29]=10'b1100_0001_00;
           lookup[30]=10'b1100_0001_00;
           lookup[31]=10'b1100_0001_00;
           lookup[32]=10'b1100_0001_00;
           lookup[33]=10'b0100_0010_00;
           lookup[34]=10'b0100_0010_00;
           lookup[35]=10'b0100_0010_00;
           lookup[36]=10'b0010_1000_00;
           lookup[37]=10'b0010_1000_00;
           lookup[38]=10'b0010_1000_00;
           lookup[39]=10'b0011_0100_00;
           lookup[40]=10'b0010_1000_00;
           lookup[41]=10'b0010_1000_00;
           lookup[42]=10'b0010_1000_00;
           lookup[43]=10'b0010_1000_00;
           lookup[44]=10'b0010_1000_00;
           lookup[45]=10'b0010_1000_00;
           lookup[46]=10'b0010_1000_00;
           lookup[47]=10'b0010_1000_00;
           lookup[48]=10'b0010_1000_00;
           lookup[49]=10'b0010_1000_00;
           lookup[50]=10'b0010_1000_00;
           lookup[51]=10'b0010_1000_00;
           lookup[52]=10'b0010_1000_00;
           lookup[53]=10'b0100_1100_00;          
           lookup[54]=10'b0100_1100_00;
           lookup[55]=10'b0100_1100_00;
           lookup[56]=10'b0100_1100_00;
           lookup[57]=10'b0100_1100_00;
           lookup[58]=10'b0100_1100_00;
           lookup[59]=10'b0100_1100_00;
           lookup[60]=10'b0010_0100_00;
           lookup[61]=10'b0010_0100_00;
           lookup[62]=10'b0010_0100_00;
           lookup[63]=10'b0010_0100_00;
   end
assign addr = divider - 1;
always @(posedge clk)
begin
    value = lookup[addr];
end     
    
    
endmodule
