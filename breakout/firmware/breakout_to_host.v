// serialized output q0: [but7, ...., but1, but0, pow0, pow1]
// serialized output q1: [din7, ...., din1, din0, pow2, pow3]

module breakout_to_host (

    // Phase-aligned frame clock and data clk (5x the speed for proper operation!)
    input   wire        i_clk,
    input   wire        i_clk_5x, // Full round robbin is i_clk_5x * 2 / 10.

    // Parallel inputs
    input   wire [7:0]  i_port,
    input   wire [7:0]  i_button,
    input   wire [3:0]  i_link_pow,

    // Serial outputs (10x i_clk due to DDR)
    output  wire        o_clk_s,
    output  wire        o_d0_s,
    output  wire        o_d1_s
);

// Latched, parallel words that we will shift out
reg [9:0] latched_d0;
reg [9:0] latched_d1;

// Shifted, parallel words
reg [9:0] shift_d0;
reg [9:0] shift_d1;

// Shift clock
//reg [9:0] shift_clk = 10'b0000011111;
reg [9:0] shift_clk = 10'b1111100000; // TODO: this is used due to inverted pins on transmitter. get rid of it with pcb fix

// Latch parallel inputs (10 bits)
always @ (posedge i_clk) begin
    latched_d0 <= {i_link_pow[1:0], i_button};
    latched_d1 <= {i_link_pow[3:2], i_port};
end

// Shift out serialized data 2 bits at a time
always @ (posedge i_clk_5x) begin

    // TODO: same as hack above
    //if (shift_clk == 10'b0000011111) begin // new sample
    if (shift_clk == 10'b1111100000) begin // new sample
       shift_d0 <= latched_d0; //10'b1010101010;
       shift_d1 <= latched_d1; //10'b0101010101;
    end else begin// 2 bits at time for DDR
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
    .OUTPUT_CLK(i_clk_5x),
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
    .OUTPUT_CLK(i_clk_5x),
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
    .OUTPUT_CLK(i_clk_5x),
    .OUTPUT_ENABLE(1'b1),
    .D_OUT_0(shift_d1[1]),
    .D_OUT_1(shift_d1[0])
);

endmodule
