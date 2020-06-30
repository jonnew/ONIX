// Data IO format
//
// 1. Breakout to host packet
//      - LVDS[0] : sys_clk
//      - LVDS[1] : [din0, din1, ... , din7], [din0, din1, ... , din7], ...
//      - LVDS[2] : [pow0, pow1, pow2, pow3, butt0, butt1, ... , butt7], [pow0, pow1, pow2, pow3, butt0, butt1, ... , butt7], ...
//      - With sys_clk = 80 MHz, din is sampled at 10 MHz per channel
//
// 2. Host to breakout packet
//      - LVDS[0] : IN_CLK
//      - LVDS[1] : [dout0, dout1, ... , dout7, led0, led1, led2, led3], [dout0, dout1, ... , dout7, led0, led1, led2, led3], ...
//      - IF IN_CLK = 120 MHz, dout is updated at 10 MHz per channel
//
// look in pins.pcf for all the pin names on the TinyFPGA BX board.
//
// This implementation is obviously a waste of bandwidth and non-optimal
// since:
//
// 1. The leds, power, buttons do not need to be polled as fast as digital
// inputs
// 2. Data is sent regardless of change in state
// 3. A synchronous transmission scheme is used instead of e.g 8b10b encoding
// and clock recovery using a single TP.
// 4. There is no error correction or even CRC sent.
//
// but its simple.
//
// TODO:
// * Host to breakout
// * Harp

//`include "pll_10_60.v"
`include "pll_10_60_2ns.v"
//`include "pll_16_60_2ns.v"
//`include "pll_16_60.v"
`include "breakout_to_host.v"
`include "host_to_breakout.v"
`include "user_io.v"
`include "harp_sync.v"

module breakout (
    input   wire            XTAL,    // 16MHz clock
    input   wire    [7:0]   D_IN,
    output  wire    [7:0]   D_OUT,
    input   wire    [1:0]   LVDS_IN,
    output  wire    [2:0]   LVDS_OUT,
    inout   wire            I2C_SCL,
    inout   wire            I2C_SDA,
    output  wire            HARP_CLK_OUT,
    output  wire            LED,   // User/boot LED next to power LED
    output  wire            USBPU, // USB pull-up resistor

    //// Test stuff
    //output   wire            I2C_SCL,
    //output   wire            I2C_SDA,
    //input   wire            PLL_SIM,
    //output  wire    [3:0]   link_led,
    //output  wire    [3:0]   link_status
);

// Internal nets
//wire reset;
reg reset;
wire [7:0] buttons;
wire [3:0] link_led;
wire [3:0] link_pow;
wire [3:0] link_status;

// PLL
wire sys_clk;
wire pll_locked;
//pll_16_60 pll_sys(XTAL, sys_clk, pll_locked); // 60 MHz sys clk
pll_10_60 pll_sys(LVDS_IN[0], sys_clk, pll_locked); // 60 MHz sys clk
//pll_16_50 pll_sys(LVDS_IN[0], sys_clk, pll_locked); // 50 MHz

//assign sys_clk = PLL_SIM;

// IO are both locked
//assign LED = pll_locked;

// One-shot reset
reg [31:0] cnt = 0;
always @ (posedge sys_clk) begin

    if (pll_locked == 1'b1) begin

        if (cnt < 500) begin
            cnt <= cnt + 1;
            reset <= 1'b1;
        end else begin
            reset <= 1'b0;
            cnt <= cnt;
        end

    end else begin
        cnt <= 0;
    end
end

// Drive USB pull-up resistor to '0' to disable USB
assign USBPU = 0;

//assign LVDS_OUT[0] = sys_clk;
//assign LVDS_OUT[1] = LVDS_IN[0];
//assign LVDS_OUT[2] = LVDS_IN[1];

// IO expander
user_io # (
    .CLK_RATE_HZ(60_000_000),
    .I2C_CLK_RATE_HZ(400_000)
) uio (
    .i_clk(sys_clk),
    .i_reset(reset),
    .i_led(link_led),
    .o_button(buttons),
    .o_link_pow(link_pow),
    .io_scl(I2C_SCL),
    .io_sda(I2C_SDA)
    //.o_state(D_OUT[4:0])
);

// Breakout to host
breakout_to_host b2h (
    .i_clk(sys_clk),
    .i_port(D_IN),
    .i_button(buttons),
    .i_link_pow(link_pow),
    .o_clk_s(LVDS_OUT[0]),
    .o_d0_s(LVDS_OUT[1]),
    .o_d1_s(LVDS_OUT[2])
);

// Host to breakout
host_to_breakout h2b (
    .i_clk(sys_clk), // Synchronous to LVDS_IN[0]
    .i_clk_s(LVDS_IN[0]),
    .i_d0_s(LVDS_IN[1]),
    .o_port(D_OUT),
    .o_led(link_led),
    .o_status(link_status)
);

// HARP
harp_sync # (
    .CLK_HZ(60_000_000)
) sync (
    .clk(sys_clk),
    .reset(reset),
    .run('b1),
    .TX(HARP_CLK_OUT),
    .LED(LED)
);
endmodule
