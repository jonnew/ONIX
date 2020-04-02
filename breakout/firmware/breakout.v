// Data IO format
//
// 1. Breakout to host packet
//      - LVDS[0] : SYS_CLK
//      - LVDS[1] : [din0, din1, ... , din7], [din0, din1, ... , din7], ...
//      - LVDS[2] : [pow0, pow1, pow2, pow3, butt0, butt1, ... , butt7], [pow0, pow1, pow2, pow3, butt0, butt1, ... , butt7], ...
//      - With SYS_CLK = 80 MHz, din is sampled at 10 MHz per channel
//
// 2. Host to breakout packet
//      - LVDS[0] : IN_CLK
//      - LVDS[1] : [dout0, dout1, ... , dout7, led0, led1, led2, led3], [dout0, dout1, ... , dout7, led0, led1, led2, led3], ...
//      - IF IN_CLK = 120 MHz, dout is updated at 10 MHz per channel
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

`include "pll_16_50.v" // Results in 10 MHz round robbin of digital inputs
`include "breakout_to_host.v"
`include "user_io.v"

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
    output  wire            USBPU  // USB pull-up resistor
);

    // Internal nets
    //wire reset;
    reg reset;
    wire [3:0] link_led;
    wire [7:0] buttons;
    wire [3:0] link_pow;

    // Temp
    wire io_clk;

    // PLL
    wire SYS_CLK;
    wire b2h_clk;
    wire pll_locked;
    pll_16_50 pll(XTAL, SYS_CLK, pll_locked); // 5x clock
    assign LED = pll_locked;

    //assign D_OUT[5] = SYS_CLK;

    // TODO: Temporary one-shot reset
    reg [31:0] cnt = 0;
    always @ (posedge SYS_CLK) begin

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

    // IO expander
    user_io # (
        .CLK_RATE_HZ(50_000_000),
        .I2C_CLK_RATE_HZ(400_000)
    ) uio (
        .i_clk(SYS_CLK),
        .i_reset(reset),
        .i_led(link_led),
        .o_button(buttons),
        .o_link_pow(link_pow),
        .io_scl(I2C_SCL),
        .io_sda(I2C_SDA)
        //.o_state(D_OUT[4:0])
    );

    // Create 1/5 SYS_CLK for breakout_to_host2
    clk_div # (
        .N(5)
    ) b2h_clk_div (
        .i_clk(SYS_CLK),
        .i_reset(reset),
        .o_clk(b2h_clk),
    );

    // Breakout to host
    // No reset, runs on PLL lock, which create SYS_CLK and b2h_clk
    breakout_to_host b2h (
        .i_clk(b2h_clk),
        .i_clk_5x(SYS_CLK),
        .i_port(D_IN),
        .i_button(buttons),
        .i_link_pow(link_pow),
        .o_clk_s(LVDS_OUT[0]),
        .o_d0_s(LVDS_OUT[1]),
        .o_d1_s(LVDS_OUT[2])
    );

    // Host to breakout
    // TODO
    //
    //// Outputs
    //breakout_output # (
    //    .RST_THRESH(10)
    //) bo (
    //    .i_clk(SYS_CLK),
    //    .i_wire_clk(LVDS_IN[0]),
    //    .i_q(LVDS_IN[1]),
    //    .o_port(D_OUT),
    //    .o_led(link_led)
    //    //.o_reset(reset)
    //);

endmodule
