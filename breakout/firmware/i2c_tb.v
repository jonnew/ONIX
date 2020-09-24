`timescale 1 ns / 100 ps

`include "wb_i2c_master_controller.v"
`include "./verilog-i2c/rtl/i2c_master_wbs_8.v"
`include "./verilog-i2c/rtl/axis_fifo.v"
`include "./verilog-i2c/rtl/i2c_master.v"

module i2c_tb();

    reg             clk;
    reg             reset;

    // WB Interface
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
    reg     [2:0]   wb_address; // TODO: this should be 3 bits, right?
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

    // I2C PHY
    wire            io_sda;
    wire            io_scl;

    // Master controller (port has same names as above)
    wb_i2c_master_controller wb_master_controller_inst (
        .i_clk(clk),
        .i_reset(reset),

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

    localparam CLK_RATE_HZ = 50e6;
    localparam I2C_CLK_RATE_HZ = 400e3;
    localparam I2C_PRESCALE = CLK_RATE_HZ / (4 * I2C_CLK_RATE_HZ); // prescale = Fclk / (FI2Cclk * 4))

    i2c_master_wbs_8 #(
        .DEFAULT_PRESCALE(1) //I2C_PRESCALE)
    //    .FIXED_PRESCALE(1), // Fix rate at default prescale
    //    .CMD_FIFO(0),
    //    .CMD_FIFO_ADDR_WIDTH(0),
    //    .WRITE_FIFO(0),
    //    .WRITE_FIFO_ADDR_WIDTH(0),
    //    .READ_FIFO(0),
    //    .READ_FIFO_ADDR_WIDTH(0),
    //i2c_master_wbs_8 //#(
    ) i2c (
        .clk,
        .rst(reset),

        .wbs_adr_i(wbs_adr_o), // Note that IO is inverted since this responding to controller
        .wbs_dat_i(wbs_dat_o),
        .wbs_dat_o(wbs_dat_i),
        .wbs_we_i(wbs_we_o),
        .wbs_stb_i(wbs_stb_o),
        .wbs_ack_o(wbs_ack_i),
        .wbs_cyc_i(wbs_cyc_o),

        .i2c_scl_i,
        .i2c_scl_o,
        .i2c_scl_t,
        .i2c_sda_i,
        .i2c_sda_o,
        .i2c_sda_t
    );

    // I2C tristate mapping
    assign i2c_scl_i = io_scl;
    assign io_scl = i2c_scl_t ? 1'bz : i2c_sda_i;
    assign i2c_sda_i = io_sda;
    assign io_sda = i2c_sda_t ? 1'bz : i2c_sda_o;

    // 50 MHz Clk
    always
    #20 clk = ~clk;

    initial begin
        $dumpfile("i2c_tb.vcd");
        $dumpvars;

        clk = 0;
        reset = 1;
        wb_data_in = 0;
        wb_address = 0;

        #100  // Lower reset
        reset = 0;

        #100  // Set the i2c address (TCA9555)
        wb_address = 2;
        wb_data_in = 8'b00100000; // All a's grounded
        wb_write = 1;
        wb_read = 0;

        // START TCA9555 REG WRITE SEQ

        #100  // Push data register with conf register 6 to set the pin direction of pins 0 thru 7
        wb_address = 4;
        wb_data_in = 8'b00000110; // Load register address
        wb_write = 1;
        wb_read = 0;

        #100  // Push the data for register 6
        wb_address = 4;
        wb_data_in = 8'b00001111; // Load register value
        wb_write = 1;
        wb_read = 0;

        #100  // Start a start, write, with NO stop sequence
        wb_address = 3;
        wb_data_in = 8'b00000101; // Issue a start and write the first data from the fifo without issueing stop
        wb_write = 1;
        wb_read = 0;

        #100  // Start a start, write, with a stop sequence
        wb_address = 3;
        wb_data_in = 8'b00010100; // Write the second followed by a stop
        wb_write = 1;
        wb_read = 0;

        // END TCA9555 REG WRITE SEQ

        #200  // Lower everything
        wb_address = 0;
        wb_data_in = 0;
        wb_write = 0;
        wb_read = 0;

        //#200  // Read the
        //wb_address = 2;
        //wb_data_in = 8'b0000001;
        //wb_write = 0;
        //wb_read = 1;

        #20000

        // START TCA9555 REG READ SEQ

        #100  // Push data register with conf register 0 to set the address to first input bank
        wb_address = 4;
        wb_data_in = 8'b00000000; // Write register address
        wb_write = 1;
        wb_read = 0;

        #100  // Start a start, write, with NO stop sequence
        wb_address = 3;
        wb_data_in = 8'b00000101; // Issue a start and write the first data from the fifo without issueing stop
        wb_write = 1;
        wb_read = 0;

        #100  // Issue a second start, this time with read, then a stop
        wb_address = 3;
        wb_data_in = 8'b00010010; // Read and stop 
        wb_write = 1;
        wb_read = 0;

        // END TCA9555 REG WRITE SEQ

        #200  // Lower everything
        wb_address = 0;
        wb_data_in = 0;
        wb_write = 0;
        wb_read = 0;


        #20000 $finish;
    end

endmodule

