// Neopixel order
// 0. PWR
// 1 - 6. Buttons 5 downto 0, in order
// 7. Harp
// 8. Lock
// 9 - 16. Digital out 7 downto 0, in order
// 17 - 24. Digital in 0 to 7, in order
// 25 - 28. HS3 downto HS0, in order
// 29 - 40. A11, A09, A07, A05, A03, A03, A01, A00, A02, A04, A06, A08, A10
//
// Matlab script used to generate the colors
//
// % Make neopixel colors with uniform brightness
// rgb = hsv(10);
// rgb = ceil(255 * 0.1 * (rgb ./ sum(rgb, 2)));
//
// % Transform into hex string
// for i = 1:size(rgb, 1)
//    disp([dec2hex(rgb(i, 1),2) dec2hex(rgb(i, 2),2) dec2hex(rgb(i, 3),2)])
// end

`include "./ws2811/ws2811.v"

// LED colors
`define PWR_RGB 'h248f24

module neopix_controller # (
    parameter CLK_RATE_HZ = 50_000_000
) (
    // Clock and reset
    input   wire            i_clk,
    input   wire            i_reset,

    // Derived from host to breakout slow word
    input   wire    [7:0]   i_ledlevel, // TODO
    input   wire    [1:0]   i_ledmode, // TODO
    input   wire    [1:0]   i_porta_status,
    input   wire    [1:0]   i_portb_status,
    input   wire    [1:0]   i_portc_status,
    input   wire    [1:0]   i_portd_status,
    input   wire    [11:0]  i_aio_dir,
    input   wire    [1:0]   i_harp_conf, // TODO
    input   wire    [15:0]  i_gpio_dir,  // TODO

    // Link power
    input   wire    [3:0]   i_link_pow,

    // HARP heartbeat
    input   wire            i_harp_hb,

    // PLL lock
    input   wire            i_pll_lock,

    // Button press state
    input   wire    [5:0]   i_button,

    // Digital IO
    input   wire    [7:0]   i_din_state,
    input   wire    [7:0]   i_dout_state,

    // Neopixel control signal
    output  wire            o_neopix
);

localparam off = 'h000000;
localparam red = 'h1A0000;
localparam org = 'h100A00;
localparam yel = 'h0C0F00;
localparam grn = 'h031800;
localparam cyn = 'h001308;
localparam lblu = 'h000D0D;
localparam blu = 'h000813;
localparam purp = 'h050016;
localparam pink = 'h0C000F;

// State machine
localparam INIT         = 0,
           RUN          = 1;

reg     [1:0]   state   = 0;

// Neopixel color state
reg [23:0] rgb [0:40];

// Currently addressed LED
wire [5:0] led_addr;

wire col_clk;

// Neopixel control
ws2811 # (
    .NUM_LEDS(41),
    .SYSTEM_CLOCK(CLK_RATE_HZ),
) driver (
    .clk(i_clk),
    .reset(i_reset),
    .address(led_addr),
    .red_in(rgb[led_addr][23:16]),
    .green_in(rgb[led_addr][15:8]),
    .blue_in(rgb[led_addr][7:0]),
    .DO(o_neopix)
);

clk_div # (
    .N(6_000_000)
) rgb_clk (
    .i_clk(i_clk),
    .i_reset(i_reset),
    .o_clk(col_clk)
);

// TODO: remove
initial begin
    rgb[00] = red;
    rgb[01] = org;
    rgb[02] = yel;
    rgb[03] = grn;
    rgb[04] = cyn;
    rgb[05] = lblu;
    rgb[06] = blu;
    rgb[07] = purp;
    rgb[08] = pink;
end


integer i;
integer j = 0;

always @ (posedge col_clk) begin

    if (i_reset) begin

    end else case(state)

        INIT: begin

            for (i = 0; i < 40; i = i + 1) begin
                if (i > j)
                    rgb[i+1] <= rgb[i];
            end

            if (j == 0)
                rgb[j] <= red;

            else if (j >= 1 && j < 7)
                rgb[j] <= blu;

            else if (j == 7)
                rgb[j] <= i_harp_hb ? grn : off;
            else if (j == 8)

                rgb[j] <= i_pll_lock ? grn : off;

            else if (j >= 9 && j < 17)
                rgb[j] <= org;

            else if (j >= 17 && j < 25)
                rgb[j] <= org;

            else if (j == 25)
                rgb[j] <= i_link_pow[3] ? red : off;
            else if (j == 26)
                rgb[j] <= i_link_pow[2] ? red : off;
            else if (j == 27)
                rgb[j] <= i_link_pow[1] ? red : off;
            else if (j == 28)
                rgb[j] <= i_link_pow[0] ? red : off;

            else if (j >= 29 && j < 41)
                rgb[j] <= blu;

            j <= j + 1;
            if (j == 40)
                state <= RUN;
        end

        RUN: begin

            for (j = 0; j < 41; j = j + 1) begin

                if (j == 0)
                    rgb[j] <= red;

                else if (j == 1)
                    rgb[j] <= i_button[5] ? org : blu;
                else if (j == 2)
                    rgb[j] <= i_button[4] ? org : blu;
                else if (j == 3)
                    rgb[j] <= i_button[3] ? org : blu;
                else if (j == 4)
                    rgb[j] <= i_button[2] ? org : blu;
                else if (j == 5)
                    rgb[j] <= i_button[1] ? org : blu;
                else if (j == 6)
                    rgb[j] <= i_button[0] ? org : blu;

                else if (j == 7)
                    rgb[j] <= i_harp_hb ? grn : off;

                else if (j == 8)
                    rgb[j] <= i_pll_lock ? grn : off;

                else if (j == 9)
                    rgb[j] <= i_din_state[7] ? org : blu;
                else if (j == 10)
                    rgb[j] <= i_din_state[6] ? org : blu;
                else if (j == 11)
                    rgb[j] <= i_din_state[5] ? org : blu;
                else if (j == 12)
                    rgb[j] <= i_din_state[4] ? org : blu;
                else if (j == 13)
                    rgb[j] <= i_din_state[3] ? org : blu;
                else if (j == 14)
                    rgb[j] <= i_din_state[2] ? org : blu;
                else if (j == 15)
                    rgb[j] <= i_din_state[1] ? org : blu;
                else if (j == 16)
                    rgb[j] <= i_din_state[0] ? org : blu;

                else if (j == 17)
                    rgb[j] <= i_dout_state[0] ? org : blu;
                else if (j == 18)
                    rgb[j] <= i_dout_state[1] ? org : blu;
                else if (j == 19)
                    rgb[j] <= i_dout_state[2] ? org : blu;
                else if (j == 20)
                    rgb[j] <= i_dout_state[3] ? org : blu;
                else if (j == 21)
                    rgb[j] <= i_dout_state[4] ? org : blu;
                else if (j == 22)
                    rgb[j] <= i_dout_state[5] ? org : blu;
                else if (j == 23)
                    rgb[j] <= i_dout_state[6] ? org : blu;
                else if (j == 24)
                    rgb[j] <= i_dout_state[7] ? org : blu;

                else if (j == 25)
                    rgb[j] <= i_porta_status == 1 ? purp : (i_link_pow[3] ? red : off);
                else if (j == 26)
                    rgb[j] <= i_portb_status == 1 ? purp : (i_link_pow[2] ? red : off);
                else if (j == 27)
                    rgb[j] <= i_portc_status == 1 ? purp : (i_link_pow[1] ? red : off);
                else if (j == 28)
                    rgb[j] <= i_portd_status == 1 ? purp : (i_link_pow[0] ? red : off);

                else if (j == 29)
                    rgb[j] <= i_aio_dir[11] ? org : blu;
                else if (j == 30)
                    rgb[j] <= i_aio_dir[9] ? org : blu;
                else if (j == 31)
                    rgb[j] <= i_aio_dir[7] ? org : blu;
                else if (j == 32)
                    rgb[j] <= i_aio_dir[5] ? org : blu;
                else if (j == 33)
                    rgb[j] <= i_aio_dir[3] ? org : blu;
                else if (j == 34)
                    rgb[j] <= i_aio_dir[1] ? org : blu;
                else if (j == 35)
                    rgb[j] <= i_aio_dir[0] ? org : blu;
                else if (j == 36)
                    rgb[j] <= i_aio_dir[2] ? org : blu;
                else if (j == 37)
                    rgb[j] <= i_aio_dir[4] ? org : blu;
                else if (j == 38)
                    rgb[j] <= i_aio_dir[6] ? org : blu;
                else if (j == 39)
                    rgb[j] <= i_aio_dir[8] ? org : blu;
                else if (j == 40)
                    rgb[j] <= i_aio_dir[10] ? org  : blu;
            end
        end

    endcase
end

endmodule
