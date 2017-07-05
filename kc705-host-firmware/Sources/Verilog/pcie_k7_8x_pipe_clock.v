// Wrapper module.

module pcie_k7_8x_pipe_clock #
(

    parameter PCIE_ASYNC_EN      = "FALSE",
    parameter PCIE_TXBUF_EN      = "FALSE",
    parameter PCIE_CLK_SHARING_EN= "FALSE",
    parameter PCIE_LANE          = 1,
    parameter PCIE_LINK_SPEED    = 3,
    parameter PCIE_REFCLK_FREQ   = 0,
    parameter PCIE_USERCLK1_FREQ = 2,
    parameter PCIE_USERCLK2_FREQ = 2,
    parameter PCIE_OOBCLK_MODE   = 1,
    parameter PCIE_DEBUG_MODE    = 0
 )
 (
   input                   CLK_CLK,
   input 		   CLK_TXOUTCLK,
   input [PCIE_LANE-1:0]   CLK_RXOUTCLK_IN,
   input 		   CLK_RST_N,
   input [PCIE_LANE-1:0]   CLK_PCLK_SEL,
   input [PCIE_LANE-1:0]   CLK_PCLK_SEL_SLAVE,
   input 		   CLK_GEN3,
   
   //---------- Output ------------------------------------
   output 		   CLK_PCLK,
   output 		   CLK_PCLK_SLAVE,
   output 		   CLK_RXUSRCLK,
   output [PCIE_LANE-1:0]  CLK_RXOUTCLK_OUT,
   output 		   CLK_DCLK,
   output 		   CLK_OOBCLK,
   output 		   CLK_USERCLK1,
   output 		   CLK_USERCLK2,
   output 		   CLK_MMCM_LOCK  
 );

   pcie_k7_vivado_pipe_clock #
     (
      .PCIE_ASYNC_EN(PCIE_ASYNC_EN),
      .PCIE_TXBUF_EN(PCIE_TXBUF_EN),
      .PCIE_LANE(PCIE_LANE),
      // synthesis translate_off
      .PCIE_LINK_SPEED(PCIE_LINK_SPEED),
      // synthesis translate_on
      .PCIE_REFCLK_FREQ(PCIE_REFCLK_FREQ),
      .PCIE_USERCLK1_FREQ(PCIE_USERCLK1_FREQ),
      .PCIE_USERCLK2_FREQ(PCIE_USERCLK2_FREQ),
      .PCIE_DEBUG_MODE(PCIE_DEBUG_MODE)
      )
   pipe_clock
     (
      .CLK_CLK(CLK_CLK),
      .CLK_TXOUTCLK(CLK_TXOUTCLK),
      .CLK_RXOUTCLK_IN(CLK_RXOUTCLK_IN),
      .CLK_RST_N(CLK_RST_N),
      .CLK_PCLK_SEL(CLK_PCLK_SEL),
      .CLK_PCLK_SEL_SLAVE(CLK_PCLK_SEL_SLAVE),
      .CLK_GEN3(CLK_GEN3),
      .CLK_PCLK(CLK_PCLK),
      .CLK_PCLK_SLAVE(CLK_PCLK_SLAVE),
      .CLK_RXUSRCLK(CLK_RXUSRCLK),
      .CLK_RXOUTCLK_OUT(CLK_RXOUTCLK_OUT),
      .CLK_DCLK(CLK_DCLK),
      .CLK_OOBCLK(CLK_OOBCLK),
      .CLK_USERCLK1(CLK_USERCLK1),
      .CLK_USERCLK2(CLK_USERCLK2),
      .CLK_MMCM_LOCK(CLK_MMCM_LOCK)
      );  

endmodule
