// A packet is a 2-bit command, 2-bit slow data, and 8-bit high-speed digital
// out:
//
//  [cmd1, cmd0, slow1, slow0, dout7, dout6, ..., dout0]
//
// Cmd          | Action
// -------------------------------------------------------------------------
// 00           | Shift slow bits into slow shift register
// 01           | Validate and move slow shift register to outputs and set inital
//              | state to [0, ..., 0, slow1, slow0]. slow1 should be the desired MSB at next
//              | cmd.
// 10           | Reserved, same as 00 currently. Don't use.
// 11           | Reset
// -------------------------------------------------------------------------
//
// With 120 MHz serial clock, this will result in periodic 10MHz dout updates.
// The full slow word is 48 bits and consists of the following elements:
//
//  MSB [acq_running,
//       acq_rst_done,
//       reserved1, reserved0,
//       ledlevel3, ledlevel2, ledlevel1, ledlevel0,
//       ledmode1, ledmod0,
//       porta_status1, porta_status0,
//       portb_status1, portb_status0,
//       portc_status1, portc_status0,
//       portd_status1, portd_status0,
//       aio_dir11, aio_dir10, ..., aio_dir0,
//       harp_conf1, harp_conf0,
//       gpio_dir15, gpio_dir14, ..., gpio_dir0] LSB
//
// which are decoded as follows:
//
// Signal       | Description
// -------------------------------------------------------------------------
// acq_running  | Host hardware run state. 0 = not running, 1 = running
// acq_rst_done | Host reset state. 0 = reset not complete, 1 = reset complete
// reserved     | NA
// ledlevel     | 4 bit register for general LED brighness. 0 = dimmest, 16 = brightest
// ledmode      | 2 bit register for LED mode. 0 = all off, 1 = only power/running, 2 = power/running, pll, harp, 3 = all on
// portx_status | 2 bit register describing the headstage port state. 3 = locked, 2 = forced_off, 1 = on but no lock, 0 = off
// aio_dir      | 12 bit register describing the direcitonality of each of the analog inputs. 0 = input, 1 = output.
// harp_conf    | 2 bit register for possible future harp configuration.
// gpio_dir     | 16 bit register for possible future digital io directionality configuration.
// -------------------------------------------------------------------------

module host_to_breakout
(
    // Local clk
    // Must be synchronous to (0 deg. phase aligned)
    // and 6x frequency of i_clk_s
    input   wire            i_clk,

    // Serial inputs
    // i_clk_s drives external PLL to create i_clk
    input   wire            i_clk_s,
    input   wire            i_d0_s,
    output  wire            o_clk_s,

    // Complete slow word
    output  reg             o_slow_valid,
    output  reg     [47:0]  o_slow_value,

    // Slow outputs (broken up version of o_slow_value)
    output reg              o_acq_running,
    output reg              o_acq_reset_done,
    output reg      [1:0]   o_reserved,
    output reg      [3:0]   o_ledlevel,
    output reg      [1:0]   o_ledmode,
    output reg      [1:0]   o_porta_status,
    output reg      [1:0]   o_portb_status,
    output reg      [1:0]   o_portc_status,
    output reg      [1:0]   o_portd_status,
    output reg      [11:0]  o_aio_dir,
    output reg      [1:0]   o_harp_conf,
    output reg      [15:0]  o_gpio_dir,

    // Host to breakout reset
    output  reg             o_reset,

    // Parallel outputs
    output  reg     [7:0]   o_port

    // Debug
    //output  reg     [1:0]   o_ddr_debug,
    //output  reg     [47:0]  o_slow_shift_debug,
    //output  reg     [11:0]  o_shift_d0_debug
);

// DDR
wire [1:0] ddr_d0_s;

// Shift register state
reg [11:0] shift_d0;
reg [47:0] slow_shift = 0;
//reg last_i_clk_s;

// TODO: remove
//assign o_ddr_debug = ~ddr_d0_s;
//assign o_slow_shift_debug = slow_shift;
//assign o_shift_d0_debug  = shift_d0;

// Initialize
initial begin
    o_acq_reset_done <= 1'b0;
    o_acq_running <= 1'b0;
    o_reserved <= 2'b00;
    o_ledlevel <= 4'b0011;
    o_ledmode <= 2'b11;
end

// Fast clock
always @ (posedge i_clk) begin
    // Shift in fast data
    shift_d0 <= {shift_d0[9:0], ddr_d0_s};
end

// Slow clock
always @ (posedge i_clk_s) begin

    // NB: shift_d0 has not yet been shifted left, so we use 2 bit
    // right-shifted indices here when indexing into it

    // Update fast output port
    o_port <= {shift_d0[5:0], ddr_d0_s};

    // Feed slow word
    slow_shift <= {slow_shift[45:0], shift_d0[7:6]};

    // Check slow word control bits
    case (shift_d0[9:8])
        2'b00 : begin // Shift slow data in
            o_reset <= 'b0;
            o_slow_valid <= 'b0;
        end
        2'b01 : begin // Set outputs
            o_reset <= 1'b0;

            o_slow_valid <= 1'b1;
            o_slow_value <= slow_shift;

            o_acq_running <= slow_shift[47];
            o_acq_reset_done <= slow_shift[46];
            o_reserved <= slow_shift[45:44];
            o_ledlevel <= slow_shift[43:40];
            o_ledmode <= slow_shift[39:38];
            o_porta_status <= slow_shift[37:36];
            o_portb_status <= slow_shift[35:34];
            o_portc_status <= slow_shift[33:32];
            o_portd_status <= slow_shift[31:30];
            o_aio_dir <= slow_shift[29:18];
            o_harp_conf <= slow_shift[17:16];
            o_gpio_dir <= slow_shift[15:0];

        end
        2'b10 : begin // Reserved
            o_reset <= 'b0;
            o_slow_valid <= 'b0;
        end
        2'b11 : begin // Signal reset
            o_reset <= 'b1;
            o_slow_valid <= 'b0;
        end
    endcase
end

// Enable DDR sampling of i_d0_s using rising and falling edge of i_clk
SB_IO # (
    .PIN_TYPE(6'b000000),
    .IO_STANDARD("SB_LVCMOS")
) d0_ddr (
    .PACKAGE_PIN(i_d0_s),
    .CLOCK_ENABLE(1'b1),
    .INPUT_CLK(i_clk),
    .D_IN_0(ddr_d0_s[1]), // rising
    .D_IN_1(ddr_d0_s[0])  // falling
);
endmodule
