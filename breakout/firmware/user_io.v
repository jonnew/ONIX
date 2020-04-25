// The TCA9555 connections are hard-coded in this module as follows
//
// P00: I_RELAY_SENSE_2
// P01: I_RELAY_SENSE_1
// P02: I_RELAY_SENSE_0
// P03: O_LED_1 (low is on)
// P04: I_BUTTON_6
// P05: I_BUTTON_2
// P06: O_LED_3 (low is on)
// P07: O_LED_2 (low is on)
// --
// P10: I_BUTTON_0
// P11: I_BUTTON_4
// P12: I_BUTTON_1
// P13: I_BUTTON_3
// P14: I_BUTTON_5
// P15: I_BUTTON_6
// P16: O_LED_0 (low is on)
// P17: I_RELAY_SENSE_3
//
// TODO:
// * RGB LED driver replaces O_LED* (breakout revision 1.3)

`include "./i2c-master/rtl/verilog/i2c_master_bit_ctrl.v"
`include "./i2c-master/rtl/verilog/i2c_master_byte_ctrl.v"
`include "./i2c-master/rtl/verilog/i2c_master_top.v"
`include "wb_i2c_master_controller.v"

module user_io # (
    parameter CLK_RATE_HZ = 16_000_000,
    parameter I2C_CLK_RATE_HZ = 400_000
) (
    // Clk and reset
    input   wire            i_clk,
    input   wire            i_reset,

    // Ins
    input   wire    [3:0]   i_led,

    // Outs
    output  reg     [7:0]   o_button,
    output  reg     [3:0]   o_link_pow,

    inout   wire            io_scl,
    inout   wire            io_sda,

    output  wire    [4:0]   o_state
);

// State machine
localparam PS0          = 0,
           PS1          = 1,
           EN_I2C       = 2,
           WR_ID        = 3,
           WR_START     = 4,
           WR_ADDR      = 5,
           WR_CONT      = 6,
           WR_VAL       = 7,
           WR_STOP      = 8,
           RD_ID        = 9,
           RD_START     = 10,
           RD_VAL       = 11,
           WB_RD_VAL    = 12,
           WAIT_WB_RD   = 13,
           WAIT_WB_WR   = 14,
           WAIT_I2C     = 15,
           I2C_STATUS   = 16;

reg     [4:0]   state   = 0;
reg     [4:0]   next_state = 0;
reg     [4:0]   next_next_state = 0;

// I2C clock prescaler
localparam PRESCALE     = CLK_RATE_HZ / (5 * I2C_CLK_RATE_HZ) - 1;
reg     [15:0]  ps_val  = PRESCALE;

// Wishbone register addresses
localparam      PRER_LO = 3'b000;
localparam      PRER_HI = 3'b001;
localparam      CTR     = 3'b010;
localparam      RXR     = 3'b011;
localparam      TXR     = 3'b011;
localparam      CR      = 3'b100;
localparam      SR      = 3'b100;

// I2C ID R/!W modifiers
localparam      RD      = 1'b1;
localparam      WR      = 1'b0;

// I2C slave ID
parameter       SADR    = 7'b0100_000;

// Wishbone signals
wire    [2:0]   wbs_adr_o;
wire    [7:0]   wbs_dat_o;
wire    [7:0]   wbs_dat_i;
wire            wbs_we_o;
wire            wbs_stb_o;
wire            wbs_ack_i;
wire            wbs_cyc_o;

// Control Signals
reg             wb_read;
reg             wb_write;

// Data Signals
reg     [7:0]   wb_data_in;
reg     [2:0]   wb_address;
wire    [7:0]   wb_data_out;

// Status Signals
wire            wb_data_out_valid;
wire            wb_done;

// I2C signals
wire            i2c_scl_i;
wire            i2c_scl_o;
wire            i2c_scl_t;
wire            i2c_sda_i;
wire            i2c_sda_o;
wire            i2c_sda_t;

// Master wishbone controller
wb_i2c_master_controller wb_master_controller_inst (
    .i_clk(i_clk),
    .i_reset(i_reset),

    // WB Interface
    .o_wbs_adr(wbs_adr_o),   // ADR_I() address
    .o_wbs_dat(wbs_dat_o),   // DAT_I() data out
    .i_wbs_dat(wbs_dat_i),   // DAT_O() data in
    .o_wbs_we(wbs_we_o),    // WE_I write enable output
    .o_wbs_stb(wbs_stb_o),   // STB_I strobe output
    .i_wbs_ack(wbs_ack_i),   // ACK_O acknowledge input
    .o_wbs_cyc(wbs_cyc_o),    // CYC_I cycle output

    // Control Signals
    .i_ren(wb_read),
    .i_wren (wb_write),

    // Data Signals
    .i_data(wb_data_in),
    .i_addr(wb_address),
    .o_data(wb_data_out),

    // Status Signals
    .o_data_val(wb_data_out_valid),
    .o_done(wb_done)
);

// I2C Master (wishbone slave)
i2c_master_top #(.ARST_LVL (0)) i2c_top2 (

    // wishbone interface
    .wb_clk_i(i_clk),
    .wb_rst_i(i_reset),
    .arst_i(1'b1), // Not used
    .wb_adr_i(wbs_adr_o),
    .wb_dat_i(wbs_dat_o),
    .wb_dat_o(wbs_dat_i),
    .wb_we_i(wbs_we_o),
    .wb_stb_i(wbs_stb_o),
    .wb_cyc_i(wbs_cyc_o),
    .wb_ack_o(wbs_ack_i),
    //.wb_inta_o(inta),

    // i2c signals
    .scl_pad_i(i2c_scl_i),
    .scl_pad_o(i2c_scl_o),
    .scl_padoen_o(i2c_scl_t),
    .sda_pad_i(i2c_sda_i),
    .sda_pad_o(i2c_sda_o),
    .sda_padoen_o(i2c_sda_t)
);

// I2C tristate mapping
assign i2c_scl_i    = io_scl;
assign io_scl       = i2c_scl_t ? 1'bz : i2c_sda_o;
assign i2c_sda_i    = io_sda;
assign io_sda       = i2c_sda_t ? 1'bz : i2c_sda_o;

// Debug
assign o_state      = state;

// Meta-states
reg [2:0] i2c_idx = 0;
reg in_write = 1'b1;

// Wishbone address and register values
reg [7:0] I2C_WR_REG_ADR [5:0];
reg [7:0] I2C_WR_REG_VAL [3:0];

// Assign constant memories
// See TCA9555 datasheet to make sense of these
initial begin
    I2C_WR_REG_ADR[0] = 8'h06; // Config reg 0, post-reset start (i2c_idx = 0)
    I2C_WR_REG_ADR[1] = 8'h07; // Config reg 1
    I2C_WR_REG_ADR[2] = 8'h02; // Output reg 0, cycle restart (i2c_idx = 2)
    I2C_WR_REG_ADR[3] = 8'h03; // Output reg 0
    I2C_WR_REG_ADR[4] = 8'h00; // Input reg 0
    I2C_WR_REG_ADR[5] = 8'h01; // Input reg 1
end

always @* begin
    I2C_WR_REG_VAL[0] = 8'b00110111; // 8'h37
    I2C_WR_REG_VAL[1] = 8'b10111111; // 8'hbf
    I2C_WR_REG_VAL[2] = {i_led[2], i_led[3], 1'b0, 1'b0, i_led[1], 1'b0, 1'b0, 1'b0}; // I2C data  TCA9555 reg 2 val
    I2C_WR_REG_VAL[3] = {1'b0, i_led[0], 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0}; // I2C data  TCA9555 reg 2 val
end

// Initialize the TCA9555 and then poll the chip using our simple single
// point-to-point wishbone master. The register read and write sequences
// here are specific to the TCA9555
always @ (posedge i_clk) begin

    if (i_reset) begin
        i2c_idx <= 0; // Reset to init
        in_write <= 1'b1;
        wb_data_in <= 0;
        wb_address <= 0;
        wb_write <= 0;
        wb_read <= 0;

        o_button <= 0;
        o_link_pow <= 0;

        state <= PS0;
        next_state <= 0;
        next_next_state <= 0;

    end else case(state)

        PS0: begin

            // Prescale low byte
            wb_address <= PRER_LO;
            wb_data_in <= ps_val[7:0];
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= PS1;
        end

        PS1: begin

            // Prescale high byte
            wb_address <= PRER_HI;
            wb_data_in <= ps_val[15:8];
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= EN_I2C;
        end

        EN_I2C : begin

            // Prescale high byte
            wb_address <= CTR;
            wb_data_in <= 8'h80; // enable i2c core
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WR_ID;
        end

        // I2C byte write
        WR_ID: begin

            wb_address <= TXR;
            wb_data_in <= {SADR, WR}; // (slave addr, write mode)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WR_START;
        end

        RD_ID: begin

            wb_address <= TXR;
            wb_data_in <= {SADR, RD}; // (slave addr, read mode)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= RD_START;
        end

        WR_START: begin

            wb_address <= CR;
            wb_data_in <= 8'h90; // (start, write)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WAIT_I2C;
            next_next_state <= WR_ADDR;

        end

        RD_START: begin

            wb_address <= CR;
            wb_data_in <= 8'h90; // (start, write)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WAIT_I2C;
            next_next_state <= RD_VAL;

        end

        WR_ADDR: begin

            wb_address <= TXR;
            wb_data_in <= I2C_WR_REG_ADR[i2c_idx]; // (memory addr, write mode)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WR_CONT;

        end

        WR_CONT: begin

            wb_address <= CR;
            wb_data_in <= 8'h10; // (write)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WAIT_I2C;

            if (in_write)
                next_next_state <= WR_VAL;
            else
                next_next_state <= RD_ID;
        end

        WR_VAL: begin

            wb_address <= TXR;
            wb_data_in <= I2C_WR_REG_VAL[i2c_idx]; // (memory val, write mode)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WR_STOP;
        end

        WR_STOP: begin

            wb_address <= CR;
            wb_data_in <= 8'h50; // (write, stop)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WAIT_I2C;
            next_next_state <= WR_ID;
            i2c_idx <= i2c_idx + 1;

            if (i2c_idx == 3)
                in_write <= 1'b0; // Go to read meta state

        end

        RD_VAL: begin

            wb_address <= CR;
            wb_data_in <= 8'h28; //8'h64; // (read, stop, nack)
            wb_write <= 1;
            state <= WAIT_WB_WR;
            next_state <= WAIT_I2C;
            next_next_state <= WB_RD_VAL;

        end

        WB_RD_VAL: begin

            wb_address <= RXR;
            wb_read <= 1;
            state <= WAIT_WB_RD;

        end

        WAIT_WB_WR: begin
            wb_write <= 0;
            if (wb_done) begin
                state <= next_state;
            end
        end

        WAIT_WB_RD: begin
            wb_read <= 0;
            if (wb_data_out_valid) begin

                // Pull data from read buffer
                if (i2c_idx == 4) begin
                    o_button[6] <= wb_data_out[4];
                    o_button[2] <= wb_data_out[5];
                    o_link_pow[2] <= wb_data_out[0];
                    o_link_pow[1] <= wb_data_out[1];
                    o_link_pow[0] <= wb_data_out[2];
                    i2c_idx <= i2c_idx + 1;
                end else begin
                    o_button[0] <= wb_data_out[0];
                    o_button[4] <= wb_data_out[1];
                    o_button[1] <= wb_data_out[2];
                    o_button[3] <= wb_data_out[3];
                    o_button[5] <= wb_data_out[4];
                    o_button[7] <= wb_data_out[5];
                    o_link_pow[3] <= wb_data_out[7];
                    in_write <= 1'b1; // Go to read meta-state
                    i2c_idx <= 2; // Reset to start of cycle
                end

                state <= WR_ID;

            end
        end

        WAIT_I2C: begin
            // Keep polling the I2C status register
            wb_address <= SR;
            wb_read <= 1;

            if (wb_data_out_valid && wb_data_out[1] == 0) begin
                state <= next_next_state;
                wb_read <= 0;
            end
            //end else begin
            //    state <= WAIT_I2C;
            //end
        end
    endcase
end

endmodule
