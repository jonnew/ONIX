// NOTES:
// This is top-level that is used for some basic post-assembly testing
//
// COMPATIBILITY:
// - fmc-host rev. 1.4
// - fmc-host rev. 1.5

// NB: Uncomment to identify implicitly-declared nets
`default_nettype none

`include "pll_16_60.v"
`include "breakout_to_host.v"
`include "host_to_breakout.v"
`include "user_io.v"
`include "harp_sync.v"
`include "neopix_controller.v"
//`include "uart_debugger.v"
`include "uart_tx.v"

module post_assembly_test (
    input   wire            XTAL,    // 16MHz clock
    output  wire    [7:0]   D_IN,    // output for testing
    output  wire    [7:0]   D_OUT,
    input   wire    [1:0]   LVDS_IN,
    output  wire    [2:0]   LVDS_OUT,
    inout   wire            I2C_SCL,
    inout   wire            I2C_SDA,
    output  wire            HARP_CLK_OUT,
    output  wire            LED,   // User/boot LED next to power LED
    output  wire            USBPU, // USB pull-up resistor
    output  wire            NEOPIX,

    // Debug
    output  wire            UART

    //// Simulation
    //output   wire            I2C_SCL,
    //output   wire            I2C_SDA,
    //input   wire             PLL_SIM
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

// Forced test state
// ---------------------------------------------------------------
assign D_OUT = 'b1111_1111; 
assign D_IN = 'b1111_1111; // D_IN is an output so we can see if all ports are high
assign acq_running = 'b1;
assign acq_reset_done = 'b1;
assign ledlevel = 'b1111; // Maximum brightness
assign ledmode = 'b11; // All LEDs active
assign porta_status = 'b11; // All ports are active
assign portb_status = 'b11;
assign portc_status = 'b11;
assign portd_status = 'b11;

// Main system clock
// ---------------------------------------------------------------
wire sys_clk;
wire pll_locked ;

// Testing
pll_16_60 pll_sys(XTAL, sys_clk, pll_locked); // 60 MHz sys clk

// Simulation
//assign reset = 0;
//assign sys_clk = PLL_SIM;

// Watchdog
// Power on & pll lock-induced reset
reg [23:0] reset_cnt = 0; // ~0.5 sec at 16 MHz
assign reset = ~reset_cnt[23];
always @(posedge XTAL)
    if (pll_locked)
        reset_cnt <= reset_cnt + reset;
    else
        reset_cnt <= 0;

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
    .i_din_state(D_IN),
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
