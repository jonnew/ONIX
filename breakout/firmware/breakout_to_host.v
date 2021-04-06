// serialized output q0: [X, X, but5, ...., but1, but0, pow0, pow1]
// serialized output q1: [din7, ...., din1, din0, pow2, pow3]

`include "clk_div.v"

module breakout_to_host (

    // 0.5 the frequency of underlying data clock
    input   wire        i_clk, // Full round robin is i_clk * 2 / 10.

    // Parallel inputs
    input   wire [7:0]  i_port,
    input   wire [5:0]  i_button,
    input   wire [3:0]  i_link_pow,

    // Clock to sample i_port
    output  wire        o_port_samp_clk,

    // Serial outputs (2x i_clk frequency due to DDR)
    output  wire        o_clk_s,
    output  wire        o_d0_s,
    output  wire        o_d1_s
);

// Shifted, parallel words
reg [9:0] shift_d0;
reg [9:0] shift_d1;

// Frame clock
reg [9:0] shift_clk = 10'b1111100000;
reg state = 0;

// Out of phase with the DDR output clock
assign o_port_samp_clk = shift_clk[4];

// Shift out serialized data and clock 2 bits at a time
always @ (posedge i_clk) begin

    if (shift_clk == 10'b0011111000) begin // new sample

        shift_d0 <= {2'b00, i_button, i_link_pow[1:0]};
        shift_d1 <= {i_port, i_link_pow[3:2]};
        state <= 1;

    end else begin // 2 bits at time for DDR

        shift_d0 <= {shift_d0[7:0], 2'b00};
        shift_d1 <= {shift_d1[7:0], 2'b00};
        state <= 0;

    end

    shift_clk <= {shift_clk[7:0], shift_clk[9:8]};
end

// Final serialized stream using DDR output drivers
SB_IO # (
    .PIN_TYPE(6'b010000),
    .IO_STANDARD("SB_LVCMOS")
) clk_ddr (
    .PACKAGE_PIN(o_clk_s),
    .CLOCK_ENABLE(1'b1),
    .OUTPUT_CLK(i_clk),
    .OUTPUT_ENABLE(1'b1),
    .D_OUT_0(shift_clk[8]),
    .D_OUT_1(shift_clk[9])
);

SB_IO # (
    .PIN_TYPE(6'b010000),
    .IO_STANDARD("SB_LVCMOS")
) d0_ddr (
    .PACKAGE_PIN(o_d0_s),
    .CLOCK_ENABLE(1'b1),
    .OUTPUT_CLK(i_clk),
    .OUTPUT_ENABLE(1'b1),
    .D_OUT_0(shift_d0[8]),
    .D_OUT_1(shift_d0[9])
);

SB_IO # (
    .PIN_TYPE(6'b010000),
    .IO_STANDARD("SB_LVCMOS")
) d1_ddr (
    .PACKAGE_PIN(o_d1_s),
    .CLOCK_ENABLE(1'b1),
    .OUTPUT_CLK(i_clk),
    .OUTPUT_ENABLE(1'b1),
    .D_OUT_0(shift_d1[8]),
    .D_OUT_1(shift_d1[9])
);

endmodule
