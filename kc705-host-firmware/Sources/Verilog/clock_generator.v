`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 27.04.2016 05:38:17
// Design Name: 
// Module Name: clock_generator
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module clock_generator(
input config_clk_in,
input clk_in,
input rst,
input [7:0] O,
input [3:0] D,
input [6:0] M,
input start_sig,
output ready,
output locked,
output clk_out
    );
    
    wire        clkfbout;
    wire        clkfbout_buf;
    wire        clk_in_buf;
    wire        clk_buf_out;
    wire        config_clk;
        
    reg         pll_den;
    reg         pll_dwe;
    reg [6:0]   pll_daddr;
    reg [15:0]  pll_di;
    wire [15:0] pll_do;
    wire        pll_drdy;
   
    wire        pll_locked;
    wire        pll_rst;
    reg         conf_rst;
       
    reg [6:0]   cmd_addr;
    reg [15:0]  cmd_data;
    reg [15:0]  cmd_mask;
    reg [15:0]  read_data;
    
    reg [2:0]   status;
    reg [3:0]   cmd_index;
    
    reg [7:0]   pll_O;
    reg [3:0]   pll_D;
    reg [6:0]   pll_M;
    
    wire [5:0]  M_high;
    wire [5:0]  M_low;
    wire        M_edge;
    
    wire [5:0]  D_high;
    wire [5:0]  D_low;
    wire        D_edge;
    
    wire [5:0]  O_high;
    wire [5:0]  O_low;
    wire        O_edge;
    
    wire [39:0] lock_lookup;
    wire [9:0]  filter_lookup;
    
    wire        start;
    reg        program_done;
    wire        cdc_busy;
    
    wire unused1, unused2, unused3, unused4, unused5;
    
    assign locked = pll_locked;
    assign pll_rst = rst | conf_rst;
    
    BUFR #(
        .BUFR_DIVIDE(2)
       ) cfgbuf
       (
       .I(config_clk_in),
       .O(config_clk),
       .CE(1'b1),
       .CLR(rst)
       );
       
       flag_cdc start_cd  (
        .clkA(config_clk_in),
        .clkB(config_clk),
        .in(start_sig),
        .out(start),
        .busy(cdc_busy));
        
        assign ready = program_done & ~cdc_busy;
    
    pll_timer_values m_values (
        .pll_value(pll_M),
        .high(M_high),
        .low(M_low),
        .w_edge(M_edge)
    );
    
    pll_timer_values d_values (
            .pll_value(pll_D),
            .high(D_high),
            .low(D_low),
            .w_edge(D_edge)
        );
        
     pll_timer_values O_values (
             .pll_value(pll_O),
             .high(O_high),
             .low(O_low),
             .w_edge(O_edge)
         );
         
     pll_lock_lookup lock_lookup_table (
        .clk(config_clk),
        .divider(pll_M),
        .value(lock_lookup)
        );
        
    pll_filter_balanced_lookup filter_lookup_table (
        .clk(config_clk),
        .divider(pll_M),
        .value(filter_lookup)
        );
    
    //main state machine
    localparam reset        = 3'h00;
    localparam wait_prog    = 3'h01;
    localparam load         = 2'h02;
    localparam read         = 3'h03;
    localparam wait_r_drdy  = 3'h04;
    localparam mask         = 3'h05;
    localparam write        = 3'h06;
    localparam wait_w_drdy   = 3'h07;
    
    always @(posedge config_clk or posedge rst)
    begin
    if (rst == 1'b1)
    begin
        cmd_index <= 4'h00;
        status <= reset;
        pll_O <= 8'h02;
        pll_D <= 4'h04;
        pll_M <= 7'h02;
        pll_den <= 1'b0;
        pll_dwe <= 1'b0;
        pll_di <= 16'b0;
        pll_daddr <= 7'b0;
        program_done <= 1'b0;
        read_data <= 16'b0;
        conf_rst = 1'b1;
    end else begin
        case (status)
            reset:
            begin
                cmd_index <= 4'h00;
                pll_O <= O;
                pll_D <= D;
                pll_M <= M;
                pll_den <= 1'b0;
                pll_dwe <= 1'b0;
                pll_di <= 16'b0;
                pll_daddr <= 7'b0;
                read_data <= 16'b0;
                program_done <= 1'b1;
                status <= wait_prog;
                conf_rst <= 1'b0;
            end
            wait_prog:
            begin
                if (start)
                begin
                    cmd_index <= 4'b00;
                    conf_rst <= 1'b1;
                    status <= load;
                    pll_O <= O;
                    pll_D <= D;
                    pll_M <= M;
                    program_done <= 1'b0;
                end
            end
            load:
            begin
                status <= read;
                pll_den <= 1'b1;
                pll_daddr <= cmd_addr;                
            end
            read:
            begin
                status <= wait_r_drdy;
                pll_den <= 1'b0;
            end
            wait_r_drdy:
            begin
                if (pll_drdy == 1'b1)
                begin
                    status <= mask;
                    read_data <= (pll_di & cmd_mask);
                end
            end
            mask:
            begin
                pll_di <= (read_data | cmd_data);
                pll_den <= 1'b1;
                pll_dwe <= 1'b1;
                status <= write;
            end
            write:
            begin
                pll_den <= 1'b0;
                pll_dwe <= 1'b0;
                status <= wait_w_drdy;
            end
            wait_w_drdy:
            begin
                if (pll_drdy)
                begin
                    if (cmd_index == 4'h0A)
                    begin
                        cmd_index <= 4'h00;
                        program_done <= 1'b1;
                        conf_rst <= 1'b0;
                        status <= wait_prog;
                    end
                    else
                    begin
                        cmd_index <= cmd_index + 4'h01;
                        status <= load;
                    end
                end
            end
            default:
                status <= reset;
        endcase
  
    end
    end
    
    always @(*)
    begin
        case(cmd_index)
            4'h00: begin //POWER 
                cmd_addr = 7'h28;
                cmd_mask = 16'h0000;
                cmd_data = 16'hFFFF;
            end
            4'h01: begin //CLKOUT0 low
                cmd_addr = 7'h08;
                cmd_mask = 16'h1000;
                cmd_data = {4'b0, O_high, O_low};
            end
            4'h02: begin //CLKOUT0 high
                cmd_addr = 7'h09;
                cmd_mask = 16'hFC00;
                cmd_data = {8'b0, O_edge, 7'b0};
            end
            4'h03: begin //DIVCLK 
                cmd_addr = 7'h16;
                cmd_mask = 16'hC000;
                cmd_data = {2'b0, D_edge, 1'b0, D_high, D_low};
            end
            4'h04: begin //CLKFBOUT low 
                cmd_addr = 7'h14;
                cmd_mask = 16'h1000;
                cmd_data = {4'b0001, M_high, M_low};
            end
            4'h05: begin //CLKFBOUT high 
                cmd_addr = 7'h15;
                cmd_mask = 16'hFC00;
                cmd_data = {8'b0, M_edge, 7'b0};
            end
            4'h06: begin //lock1
                cmd_addr = 7'h18;
                cmd_mask = 16'hFC00;
                cmd_data = {6'b0, lock_lookup[29:20]};
            end
            4'h07: begin //lock2 
                cmd_addr = 7'h19;
                cmd_mask = 16'h8000;
                cmd_data = {1'b0, lock_lookup[34:30], lock_lookup[9:0]};
            end
            4'h08: begin //lock3 
                cmd_addr = 7'h1A;
                cmd_mask = 16'h8000;
                cmd_data = {1'b0, lock_lookup[39:35], lock_lookup[19:10]};
            end
            4'h09: begin //filter1 
                cmd_addr = 7'h4e;
                cmd_mask = 16'h66FF;
                cmd_data = {filter_lookup[9], 2'b0, filter_lookup[8:7], 2'b0, filter_lookup[6], 8'b0};
            end
            4'h0A: begin //filter2 
                cmd_addr = 7'h4f;
                cmd_mask = 16'h666F;
                cmd_data = {filter_lookup[5], 2'b0, filter_lookup[4:3], 2'b0, filter_lookup[2:1], 2'b0, filter_lookup[0], 4'b0};
            end
            default:
            begin
                cmd_addr = 7'h00;
                cmd_data = 16'h00;
                cmd_mask = 16'h00;
            end
        endcase
    end

PLLE2_ADV
  #(.BANDWIDTH            ("OPTIMIZED"),
    .COMPENSATION         ("ZHOLD"),
    .DIVCLK_DIVIDE        (4),
    .CLKFBOUT_MULT        (42),
    .CLKFBOUT_PHASE       (0.000),
    .CLKOUT0_DIVIDE       (25),
    .CLKOUT0_PHASE        (0.000),
    .CLKOUT0_DUTY_CYCLE   (0.500),
    .CLKIN1_PERIOD        (5.0))
  plle2_adv_inst
    // Output clocks
   (
    .CLKFBOUT            (clkfbout),
    .CLKOUT0             (clk_buf_out),
    .CLKOUT1             (unused1),
    .CLKOUT2             (unused2),
    .CLKOUT3             (unused3),
    .CLKOUT4             (unused4),
    .CLKOUT5             (unused5),
     // Input clock control
    .CLKFBIN             (clkfbout_buf),
    .CLKIN1              (clk_in_buf),
    .CLKIN2              (1'b0),
     // Tied to always select the primary input clock
    .CLKINSEL            (1'b1),
    // Ports for dynamic reconfiguration
    .DADDR               (pll_daddr),
    .DCLK                (config_clk),
    .DEN                 (pll_den),
    .DI                  (pll_di),
    .DO                  (pll_do),
    .DRDY                (pll_drdy),
    .DWE                 (pll_dwe),
    // Other control and status signals
    .LOCKED              (pll_locked),
    .PWRDWN              (1'b0),
    .RST                 (pll_rst));
    
    BUFH clkin_buf (
        .O(clk_in_buf),
        .I(clk_in)
    );
  //assign clk_in_buf = clk_in;
    
    BUFG clkfb_buf (
        .O(clkfbout_buf),
        .I(clkfbout)
    );
    
    BUFG clkout_buf (
        .O(clk_out),
        .I(clk_buf_out)
    );
    
endmodule
