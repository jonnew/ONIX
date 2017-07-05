`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 28.04.2016 03:05:18
// Design Name: 
// Module Name: pll_timer_values
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: Generates the high, low and edge values for PLL configuration with a fixes duty cycle of 50%
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module pll_timer_values(
       input [7:0] pll_value,
       output [5:0] high,
       output [5:0] low,
       output w_edge
    );
    
    wire [5:0]  temp;
    wire        temp_edge;
    
    assign temp_edge = pll_value[0];
    assign temp = pll_value[6:1];
    
    assign low = pll_value[6:1] + pll_value[0];
    assign high = (temp == 6'b0) ? 6'h01 : temp;
    assign w_edge = (temp == 6'b0) ? 1'b0 : temp_edge;
    
endmodule
