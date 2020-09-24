/**
 * PLL configuration
 *
 * This Verilog module was generated automatically
 * using the icepll tool from the IceStorm project.
 * Use at your own risk.
 *
 * Given input frequency:        10.000 MHz
 * Requested output frequency:   60.000 MHz
 * Achieved output frequency:    60.000 MHz
 */

module pll_10_60_2ns (
    input  clock_in,
    output clock_out,
    output locked
);

SB_PLL40_CORE # (
    .FEEDBACK_PATH("DELAY"),
    .DIVR(4'b0000),     // DIVR =  0
    .DIVF(7'b0000101),  // DIVF =  5
    .DIVQ(3'b100),      // DIVQ =  4
    .FILTER_RANGE(3'b001),  // FILTER_RANGE = 1
    .DELAY_ADJUSTMENT_MODE_FEEDBACK("FIXED"),
    .FDA_FEEDBACK(15), // Empically, we need to shift the output back by 2 ns or so
    .FDA_RELATIVE(0)
) uut (
    .LOCK(locked),
    .RESETB(1'b1),
    .BYPASS(1'b0),
    .REFERENCECLK(clock_in),
    .PLLOUTCORE(clock_out)
);

endmodule
