/**
 * PLL configuration
 *
 * This Verilog module was generated automatically
 * using the icepll tool from the IceStorm project.
 * Use at your own risk.
 *
 * Given input frequency:        16.000 MHz
 * Requested output frequency:   60.000 MHz
 * Achieved output frequency:    64.000 MHz
 */

module pll_16_60_2ns(
    input  clock_in,
    output clock_out,
    output locked
    );

// Phase delayed by ~2ns to allow sampling in middle of underlying 120
// Mhz clock using DDR
SB_PLL40_CORE #(
        .FEEDBACK_PATH("PHASE_AND_DELAY"),
        .DIVR(4'b0000),     // DIVR =  0
        .DIVF(7'b0000011),  // DIVF =  3
        .DIVQ(3'b100),      // DIVQ =  4
        .FILTER_RANGE(3'b001),   // FILTER_RANGE = 1
        .DELAY_ADJUSTMENT_MODE_FEEDBACK("FIXED"),
        .FDA_FEEDBACK(0), //12)
        .PLLOUT_SELECT("SHIFTREG_0deg")
    ) uut (
        .LOCK(locked),
        .RESETB(1'b1),
        .BYPASS(1'b0),
        .REFERENCECLK(clock_in),
        .PLLOUTCORE(clock_out)
        );

endmodule
