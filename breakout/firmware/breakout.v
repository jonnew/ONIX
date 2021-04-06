// NOTES:
// Facts about this design:
//
// 1. It is meant, insofar as possible, to be a zero-latency "view" into the
// hardware on the breakout board that is controlled by the host.
//
// 2. The clock governing board functions is derived from an LVDS line coming
// from the host. The PLL is locked to this clock for data transmission. Data
// coming to the breakout board is synchronous to the LVDS clock and data
// going out of the breakout board is synchronous to the PLL-generated clock.
// Have a look at host_to_breakout.v and breakout_to_host.v for
// descriptions of data packet format.
//
// 3. There is no error correction or CRC, currently.
//
// COMPATIBILITY:
// - fmc-host rev. 1.4
// - fmc-host rev. 1.5

// NB: Used to identify implicitly-declared nets
`default_nettype none

`include "pll_10_60_2ns.v"
//`include "pll_16_60.v"
`include "breakout_to_host.v"
`include "host_to_breakout.v"
`include "user_io.v"
`include "harp_sync.v"
`include "neopix_controller.v"
//`include "uart_debugger.v"
`include "uart_tx.v"

module breakout (
    input   wire            XTAL,    // 16MHz clock
    input   wire            D_IN0,   // Manual SB_IO means I can't use array
    input   wire            D_IN1,
    input   wire            D_IN2,
    input   wire            D_IN3,
    input   wire            D_IN4,
    input   wire            D_IN5,
    input   wire            D_IN6,
    input   wire            D_IN7,
    output  wire    [7:0]   D_OUT,
    input   wire    [1:0]   LVDS_IN,
    output  wire    [2:0]   LVDS_OUT,
    output  wire            HARP_CLK_OUT,
    output  wire            LED,    // User/boot LED next to power LED
    output  wire            USBPU,  // USB pull-up resistor
    output  wire            NEOPIX,
`ifndef SIMULATION
    inout   wire            I2C_SCL,
    inout   wire            I2C_SDA,
`else
    input   wire            PLL_SIM,
    output  wire            I2C_SCL,
    output  wire            I2C_SDA,
`endif
    output  wire            UART    // For debug
);


// Internal nets
reg reset;
wire [5:0] buttons;
wire [3:0] link_pow;
wire harp_hb;

// Host to breakout slow word results
//wire slow_word_valid;
//wire [47:0] slow_word;
wire acq_running;
wire acq_reset_done;
wire [3:0] ledlevel;
wire [1:0] ledmode;
wire [1:0] porta_status;
wire [1:0] portb_status;
wire [1:0] portc_status;
wire [1:0] portd_status;
wire [11:0] aio_dir;
wire [1:0] harp_conf;
wire [15:0] gpio_dir;

// DIN sample clock
reg d_in_clk;

// D_INs with pullup applied
wire [7:0] d_in_pu;

// Main system clock
// ---------------------------------------------------------------
wire sys_clk;
wire pll_locked ;

`ifndef SIMULATION

pll_10_60_2ns pll_sys(LVDS_IN[0], sys_clk, pll_locked); // 60 MHz sys clk

// Testing
//pll_16_60 pll_sys(XTAL, sys_clk, pll_locked); // 60 MHz sys clk

// Watchdog
// Power on & pll lock-induced reset
reg [23:0] reset_cnt = 0; // ~0.5 sec at 16 MHz
assign reset = ~reset_cnt[23];
always @(posedge XTAL)
    if (pll_locked)
        reset_cnt <= reset_cnt + reset;
    else
        reset_cnt <= 0;

`else

assign reset = 0;
assign sys_clk = PLL_SIM;

`endif

// IO expander
// ---------------------------------------------------------------
user_io # (
    .CLK_RATE_HZ(60_000_000),
    .I2C_CLK_RATE_HZ(400_000)
) uio (
    .i_clk(sys_clk),
    .i_reset(reset),
    .i_porta_status(porta_status),
    .i_portb_status(portb_status),
    .i_portc_status(portc_status),
    .i_portd_status(portd_status),
    .o_button(buttons),
    .o_link_pow(link_pow),
    .io_scl(I2C_SCL),
    .io_sda(I2C_SDA)
);

// Breakout to host
// ---------------------------------------------------------------
breakout_to_host b2h (
    .i_clk(sys_clk),
    .i_port(d_in_pu),
    .i_button(buttons),
    .i_link_pow(link_pow),
    .o_port_samp_clk(d_in_clk),
    .o_clk_s(LVDS_OUT[0]),
    .o_d0_s(LVDS_OUT[1]),
    .o_d1_s(LVDS_OUT[2])
);

// Host to breakout
// ---------------------------------------------------------------
host_to_breakout h2b (
    .i_clk(sys_clk),        // Synchronous to LVDS_IN[0]
    .i_clk_s(LVDS_IN[0]),
    .i_d0_s(LVDS_IN[1]),
    .o_port(D_OUT),
    //.o_reset(TODO),       // Not convinced this needed or good
    //.o_reserved(TODO),
    .o_acq_running(acq_running),
    .o_acq_reset_done(acq_reset_done),
    .o_ledlevel(ledlevel),
    .o_ledmode(ledmode),
    .o_porta_status(porta_status),
    .o_portb_status(portb_status),
    .o_portc_status(portc_status),
    .o_portd_status(portd_status),
    .o_aio_dir(aio_dir),
    .o_harp_conf(harp_conf),
    .o_gpio_dir(gpio_dir)
    //.o_slow_valid(slow_word_valid),
    //.o_slow_value(slow_word)
);

// HARP
// ---------------------------------------------------------------
harp_sync # (
    .CLK_RATE_HZ(60_000_000)
) sync (
    .clk(sys_clk),
    .reset(reset),
    .run('b1),
    .TX(HARP_CLK_OUT),
    .LED(harp_hb)
);

// Neopixel control
// ---------------------------------------------------------------
neopix_controller # (
    .CLK_RATE_HZ(60_000_000)
) neopix (
    .i_clk(sys_clk),
    .i_reset(reset),
    .i_harp_hb(harp_hb),
    .i_link_pow(link_pow),
    .i_button(buttons),
    .i_din_state(d_in_pu),
    .i_dout_state(D_OUT),
    .i_acq_running(acq_running),
    .i_acq_reset_done(acq_reset_done),
    .i_ledlevel(ledlevel),
    .i_ledmode(ledmode),
    .i_porta_status(porta_status),
    .i_portb_status(portb_status),
    .i_portc_status(portc_status),
    .i_portd_status(portd_status),
    .i_aio_dir(aio_dir),
    .i_harp_conf(harp_conf),
    .i_gpio_dir(gpio_dir),
    .o_neopix(NEOPIX)
);

// IO settings
// ---------------------------------------------------------------

// Only indicate programming mode (important for in the field programming)
assign LED = 0;

// Drive USB pull-up resistor to '0' to disable USB
assign USBPU = 0;

// Digital inputs are sampled on the falling edge of lvds_out
// The result is packed on the rising edge to avoid metastability

// Enable pullups on the digital input port
SB_IO # (
    .PIN_TYPE(6'b 0000_00),
    .PULLUP(1'b1)
) din0 (
    .PACKAGE_PIN(D_IN0),
    .D_IN_0(d_in_pu[0]),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(d_in_clk)
);

SB_IO # (
    .PIN_TYPE(6'b 0000_00),
    .PULLUP(1'b1)
) din1 (
    .PACKAGE_PIN(D_IN1),
    .D_IN_0(d_in_pu[1]),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(d_in_clk)
);

SB_IO # (
    .PIN_TYPE(6'b 0000_00),
    .PULLUP(1'b1)
) din2 (
    .PACKAGE_PIN(D_IN2),
    .D_IN_0(d_in_pu[2]),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(d_in_clk)
);

SB_IO # (
    .PIN_TYPE(6'b 0000_00),
    .PULLUP(1'b1)
) din3 (
    .PACKAGE_PIN(D_IN3),
    .D_IN_0(d_in_pu[3]),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(d_in_clk)
);

SB_IO # (
    .PIN_TYPE(6'b 0000_00),
    .PULLUP(1'b1)
) din4 (
    .PACKAGE_PIN(D_IN4),
    .D_IN_0(d_in_pu[4]),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(d_in_clk)
);

SB_IO # (
    .PIN_TYPE(6'b 0000_00),
    .PULLUP(1'b1)
) din5 (
    .PACKAGE_PIN(D_IN5),
    .D_IN_0(d_in_pu[5]),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(d_in_clk)
);

SB_IO # (
    .PIN_TYPE(6'b 0000_00),
    .PULLUP(1'b1)
) din6 (
    .PACKAGE_PIN(D_IN6),
    .D_IN_0(d_in_pu[6]),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(d_in_clk)
);

SB_IO # (
    .PIN_TYPE(6'b 0000_00),
    .PULLUP(1'b1)
) din7 (
    .PACKAGE_PIN(D_IN7),
    .D_IN_0(d_in_pu[7]),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(d_in_clk)
);

// Debugger
// ---------------------------------------------------------------
//uart_debugger # (
//    .DATA_BYTES(6),
//    .CLK_RATE_HZ(60_000_000),
//    .DEAD_CLKS(6000)
//) debugger (
//    .i_clk(sys_clk),
//    .i_reset(reset),
//    .i_data_valid(slow_word_valid),
//    .i_data(slow_word),
//    .o_uart_tx(UART)
//);
endmodule
