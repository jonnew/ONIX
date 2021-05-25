module wb_i2c_master_controller (
    input     wire          i_clk,
    input     wire          i_reset,

    // WB Interface (directions are from slave)
    output    reg [2:0]     o_wbs_adr,   // ADR_I() address
    output    reg [7:0]     o_wbs_dat,   // DAT_I() data out
    input     wire [7:0]    i_wbs_dat,   // DAT_O() data in
    output    reg           o_wbs_we,    // WE_I write enable output
    output    reg           o_wbs_stb,   // STB_I strobe output
    input     wire          i_wbs_ack,   // ACK_O acknowledge input
    output    reg           o_wbs_cyc,   // CYC_I cycle output

    // Control Signals
    input     wire          i_ren,
    input     wire          i_wren,

    // Data Signals
    input     wire [7:0]    i_data,
    input     wire [2:0]    i_addr,
    output    wire [7:0]    o_data,

    // Status Signals
    output    wire           o_data_val,
    output    wire           o_done
);

localparam WB_IDLE            = 0;
localparam WB_WAIT_FOR_ACK    = 1;
reg [1:0]   state = 0;

// Data and status outputs
assign o_data = i_wbs_dat;
assign o_data_val = o_wbs_we == 0 ? i_wbs_ack : 1'b0;
assign o_done = i_wbs_ack;

// WB State Machine
always @ (posedge i_clk) begin

    if (i_reset == 1'b1) begin

        // SM
        state <= WB_IDLE;

        // Status
        o_wbs_dat <= 0;

        // WB Signals
        o_wbs_we <= 1'b0;
        o_wbs_stb <= 1'b0;
        o_wbs_cyc <= 1'b0;
        o_wbs_adr <= 0;
        o_wbs_dat <= 0;

    end else begin

        case (state)

            WB_IDLE: begin // Does Nothing. Waits for wb_read or wb_write
                o_wbs_stb <= 1'b0;
                o_wbs_cyc <= 1'b0;

                // Writes data to a WB register
                if (i_wren) begin
                    state <= WB_WAIT_FOR_ACK;
                    o_wbs_adr <= i_addr;
                    o_wbs_dat <= i_data;
                    o_wbs_we <= 1'b1;
                    o_wbs_stb <= 1'b1;
                    o_wbs_cyc <= 1'b1;
                end

                // Reads data to a WB register
                if (i_ren) begin
                    state <= WB_WAIT_FOR_ACK;
                    o_wbs_adr <= i_addr;
                    o_wbs_dat <= i_data;
                    o_wbs_we <= 1'b0;
                    o_wbs_stb <= 1'b1;
                    o_wbs_cyc <= 1'b1;
                end
            end

            WB_WAIT_FOR_ACK: begin // Waits for a RD Ack
                if (i_wbs_ack) begin
                    o_wbs_stb <= 1'b0;
                    o_wbs_cyc <= 1'b0;
                    state <= WB_IDLE;
                end
            end

        endcase
    end
end

endmodule
