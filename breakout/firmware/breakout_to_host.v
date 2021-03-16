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

    // Serial outputs (2x i_clk frequency due to DDR)
    output  wire        o_clk_s,
    output  wire        o_d0_s,
    output  wire        o_d1_s
);

// Shifted, parallel words
reg [9:0] shift_d0;
reg [9:0] shift_d1;

// Frame clock
wire frame_clk;

// Shift clock
reg [9:0] shift_clk = 10'b0000011111;

// Shift out serialized data and clock 2 bits at a time
always @ (posedge i_clk) begin

    if (shift_clk == 10'b0001111100) begin // new sample
        shift_d0 <= {i_link_pow[1:0], i_button[5:0], 2'b00};
        shift_d1 <= {i_link_pow[3:2], i_port[7:0]};

    end else begin // 2 bits at time for DDR
        shift_d0 <= {2'b00, shift_d0[9:2]};
        shift_d1 <= {2'b00, shift_d1[9:2]};
    end

    shift_clk <= {shift_clk[1:0], shift_clk[9:2]};
end

// Finally, created serialized using DDR output drivers
SB_IO # (
    .PIN_TYPE(6'b010000),
    .IO_STANDARD("SB_LVCMOS")
) clk_ddr (
    .PACKAGE_PIN(o_clk_s),
    .CLOCK_ENABLE(1'b1),
    .OUTPUT_CLK(i_clk),
    .OUTPUT_ENABLE(1'b1),
    .D_OUT_0(shift_clk[1]),
    .D_OUT_1(shift_clk[0])
);

SB_IO # (
    .PIN_TYPE(6'b010000),
    .IO_STANDARD("SB_LVCMOS")
) d0_ddr (
    .PACKAGE_PIN(o_d0_s),
    .CLOCK_ENABLE(1'b1),
    .OUTPUT_CLK(i_clk),
    .OUTPUT_ENABLE(1'b1),
    .D_OUT_0(shift_d0[1]),
    .D_OUT_1(shift_d0[0])
);

SB_IO # (
    .PIN_TYPE(6'b010000),
    .IO_STANDARD("SB_LVCMOS")
) d1_ddr (
    .PACKAGE_PIN(o_d1_s),
    .CLOCK_ENABLE(1'b1),
    .OUTPUT_CLK(i_clk),
    .OUTPUT_ENABLE(1'b1),
    .D_OUT_0(shift_d1[1]),
    .D_OUT_1(shift_d1[0])
);

endmodule
