`timescale 10ns/1ns

module ws2811_tb;
   reg clk;
   reg reset;

   wire [2:0] address;
   reg [7:0]  red;
   reg [7:0]  green;
   reg [7:0]  blue;
   
   wire       DO;
   
   ws2811
     #(
       .NUM_LEDS(8),
       .SYSTEM_CLOCK(100000000)
       ) driver
       (
        .clk(clk),
        .reset(reset),
        .address(address),
        .red_in(red),
        .green_in(green),
        .blue_in(blue),
        .DO(DO)
      );
   
   
   initial begin
      $dumpfile("out.vcd");
      $dumpvars(0, ws2811_tb);
      
      clk = 0;
      reset = 1;

      red = 8'hFF;
      green = 8'hAA;
      blue = 8'h00;
      
      #50 reset = 0;
      #1000000 $finish();
   end

   always clk = #0.5 ~clk;
   
endmodule
