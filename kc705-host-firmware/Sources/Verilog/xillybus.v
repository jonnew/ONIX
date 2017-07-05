`timescale 1ns / 10ps

module xillybus(PCIE_TX_P, PCIE_TX_N, PCIE_RX_P, PCIE_RX_N, PCIE_REFCLK_P,
  PCIE_REFCLK_N, PCIE_PERST_B_LS, bus_clk, quiesce, GPIO_LED,
  user_r_neural_data_32_rden, user_r_neural_data_32_data,
  user_r_neural_data_32_empty, user_r_neural_data_32_eof,
  user_r_neural_data_32_open, user_r_control_regs_16_rden,
  user_r_control_regs_16_data, user_r_control_regs_16_empty,
  user_r_control_regs_16_eof, user_r_control_regs_16_open,
  user_w_control_regs_16_wren, user_w_control_regs_16_data,
  user_w_control_regs_16_full, user_w_control_regs_16_open,
  user_control_regs_16_addr, user_control_regs_16_addr_update,
  user_r_status_regs_16_rden, user_r_status_regs_16_data,
  user_r_status_regs_16_empty, user_r_status_regs_16_eof,
  user_r_status_regs_16_open, user_status_regs_16_addr,
  user_status_regs_16_addr_update, user_w_auxcmd1_membank_16_wren,
  user_w_auxcmd1_membank_16_data, user_w_auxcmd1_membank_16_full,
  user_w_auxcmd1_membank_16_open, user_auxcmd1_membank_16_addr,
  user_auxcmd1_membank_16_addr_update, user_w_auxcmd2_membank_16_wren,
  user_w_auxcmd2_membank_16_data, user_w_auxcmd2_membank_16_full,
  user_w_auxcmd2_membank_16_open, user_auxcmd2_membank_16_addr,
  user_auxcmd2_membank_16_addr_update, user_w_auxcmd3_membank_16_wren,
  user_w_auxcmd3_membank_16_data, user_w_auxcmd3_membank_16_full,
  user_w_auxcmd3_membank_16_open, user_auxcmd3_membank_16_addr,
  user_auxcmd3_membank_16_addr_update);

  input [7:0] PCIE_RX_P;
  input [7:0] PCIE_RX_N;
  input  PCIE_REFCLK_P;
  input  PCIE_REFCLK_N;
  input  PCIE_PERST_B_LS;
  input [31:0] user_r_neural_data_32_data;
  input  user_r_neural_data_32_empty;
  input  user_r_neural_data_32_eof;
  input [15:0] user_r_control_regs_16_data;
  input  user_r_control_regs_16_empty;
  input  user_r_control_regs_16_eof;
  input  user_w_control_regs_16_full;
  input [15:0] user_r_status_regs_16_data;
  input  user_r_status_regs_16_empty;
  input  user_r_status_regs_16_eof;
  input  user_w_auxcmd1_membank_16_full;
  input  user_w_auxcmd2_membank_16_full;
  input  user_w_auxcmd3_membank_16_full;
  output [7:0] PCIE_TX_P;
  output [7:0] PCIE_TX_N;
  output  bus_clk;
  output  quiesce;
  output [3:0] GPIO_LED;
  output  user_r_neural_data_32_rden;
  output  user_r_neural_data_32_open;
  output  user_r_control_regs_16_rden;
  output  user_r_control_regs_16_open;
  output  user_w_control_regs_16_wren;
  output [15:0] user_w_control_regs_16_data;
  output  user_w_control_regs_16_open;
  output [4:0] user_control_regs_16_addr;
  output  user_control_regs_16_addr_update;
  output  user_r_status_regs_16_rden;
  output  user_r_status_regs_16_open;
  output [4:0] user_status_regs_16_addr;
  output  user_status_regs_16_addr_update;
  output  user_w_auxcmd1_membank_16_wren;
  output [15:0] user_w_auxcmd1_membank_16_data;
  output  user_w_auxcmd1_membank_16_open;
  output [15:0] user_auxcmd1_membank_16_addr;
  output  user_auxcmd1_membank_16_addr_update;
  output  user_w_auxcmd2_membank_16_wren;
  output [15:0] user_w_auxcmd2_membank_16_data;
  output  user_w_auxcmd2_membank_16_open;
  output [15:0] user_auxcmd2_membank_16_addr;
  output  user_auxcmd2_membank_16_addr_update;
  output  user_w_auxcmd3_membank_16_wren;
  output [15:0] user_w_auxcmd3_membank_16_data;
  output  user_w_auxcmd3_membank_16_open;
  output [15:0] user_auxcmd3_membank_16_addr;
  output  user_auxcmd3_membank_16_addr_update;
  wire  trn_reset_n;
  wire  trn_lnk_up_n;
  wire  s_axis_tx_tready;
  wire [63:0] s_axis_tx_tdata;
  wire [7:0] s_axis_tx_tkeep;
  wire  s_axis_tx_tlast;
  wire  s_axis_tx_tvalid;
  wire [63:0] m_axis_rx_tdata;
  wire [7:0] m_axis_rx_tkeep;
  wire  m_axis_rx_tlast;
  wire  m_axis_rx_tvalid;
  wire  m_axis_rx_tready;
  wire  cfg_interrupt_n;
  wire  cfg_interrupt_rdy_n;
  wire [7:0] cfg_bus_number;
  wire [4:0] cfg_device_number;
  wire [2:0] cfg_function_number;
  wire [15:0] cfg_dcommand;
  wire [15:0] cfg_lcommand;
  wire [15:0] cfg_dstatus;
  wire  trn_rerrfwd_n;
  wire [7:0] trn_fc_cplh;
  wire [11:0] trn_fc_cpld;
  wire  trn_terr_drop_n;
  wire  pcie_ref_clk;
  wire  user_reset_out;
  wire  user_lnk_up;
  wire  cfg_interrupt_rdy;
  wire  tx_err_drop;
  wire [21:0] m_axis_rx_tuser;
  wire  PIPE_PCLK_IN;
  wire  PIPE_RXUSRCLK_IN;
  wire [7:0] PIPE_RXOUTCLK_IN;
  wire  PIPE_DCLK_IN;
  wire  PIPE_USERCLK1_IN;
  wire  PIPE_USERCLK2_IN;
  wire  PIPE_MMCM_LOCK_IN;
  wire  PIPE_TXOUTCLK_OUT;
  wire [7:0] PIPE_RXOUTCLK_OUT;
  wire [7:0] PIPE_PCLK_SEL_OUT;
  wire  PIPE_GEN3_OUT;
  wire  PIPE_OOBCLK_IN;

  // Wires used for external clocking connectivity

   IBUFDS_GTE2 pcieclk_ibuf (.O(pcie_ref_clk), .ODIV2(),
			     .I(PCIE_REFCLK_P), .IB(PCIE_REFCLK_N),
			     .CEB(1'b0));
   
   assign 	     trn_reset_n = !user_reset_out;
   assign 	     trn_lnk_up_n = !user_lnk_up;
   assign 	     cfg_interrupt_rdy_n = !cfg_interrupt_rdy;
   assign 	     trn_terr_drop_n = tx_err_drop;
   assign 	     trn_rerrfwd_n = !m_axis_rx_tuser[1];
   
   pcie_k7_8x #( .PCIE_EXT_CLK("TRUE") ) pcie
     (
     .pci_exp_txp( PCIE_TX_P ),
     .pci_exp_txn( PCIE_TX_N ),
     .pci_exp_rxp( PCIE_RX_P ),
     .pci_exp_rxn( PCIE_RX_N ),

     .user_clk_out(bus_clk),
     .user_reset_out(user_reset_out),
     .user_lnk_up(user_lnk_up),

     .s_axis_tx_tready( s_axis_tx_tready ),
     .s_axis_tx_tdata( s_axis_tx_tdata ),
     .s_axis_tx_tkeep( s_axis_tx_tkeep ),
     .s_axis_tx_tuser( 4'd0 ), // No error fwd, no discontinue, no streaming
     .s_axis_tx_tlast( s_axis_tx_tlast ),
     .s_axis_tx_tvalid( s_axis_tx_tvalid ),
     .tx_cfg_gnt(1'b1), // Always grant configuration transmit
     .tx_err_drop(tx_err_drop),
      
     .m_axis_rx_tdata( m_axis_rx_tdata ),
     .m_axis_rx_tkeep( m_axis_rx_tkeep ),
     .m_axis_rx_tlast( m_axis_rx_tlast ),
     .m_axis_rx_tvalid( m_axis_rx_tvalid ),
     .m_axis_rx_tready( m_axis_rx_tready ),
     .m_axis_rx_tuser(m_axis_rx_tuser),
     .rx_np_ok( 1'b1 ),

     .fc_cpld(trn_fc_cpld), // Completion Data credits
     .fc_cplh(trn_fc_cplh), // Completion Header credits
     .fc_npd(  ),
     .fc_nph(  ),
     .fc_pd(  ),
     .fc_ph(  ),
     .fc_sel(3'd0), // Receive credit available space

     .cfg_mgmt_di( 32'd0 ),
     .cfg_mgmt_byte_en( 4'h0 ),
     .cfg_mgmt_dwaddr( 10'd0 ),
     .cfg_mgmt_wr_en( 1'b0 ),
     .cfg_mgmt_rd_en( 1'b0 ),
     .cfg_mgmt_wr_rw1c_as_rw( 1'b0 ),
     .cfg_mgmt_wr_readonly( 1'b0 ),
      
     .cfg_err_cor(1'b0),
     .cfg_err_ur(1'b0),
     .cfg_err_ecrc(1'b0),
     .cfg_err_cpl_timeout(1'b0),
     .cfg_err_cpl_abort(1'b0),
     .cfg_err_cpl_unexpect(1'b0),
     .cfg_err_posted(1'b0),
     .cfg_err_tlp_cpl_header(48'd0),
     .cfg_err_locked(1'b0),
     .cfg_err_poisoned(1'b0),
     .cfg_err_malformed(1'b0),
     .cfg_err_acs(1'b0), // Undocumented but ignored anyhow
     .cfg_err_atomic_egress_blocked(1'b0),
     .cfg_err_mc_blocked(1'b0),
     .cfg_err_internal_uncor(1'b0),
     .cfg_err_internal_cor(1'b0),
     .cfg_err_norecovery(1'b0),
     .cfg_err_aer_headerlog(128'd0), // No errors are reported, so ignored

     .cfg_interrupt( !cfg_interrupt_n ),
     .cfg_interrupt_rdy( cfg_interrupt_rdy ),

     .cfg_interrupt_assert(1'b0),
     .cfg_interrupt_di(8'd0), // Single MSI anyhow

     .cfg_pm_wake(1'b0),
     .cfg_trn_pending(1'b0),

     .cfg_bus_number( cfg_bus_number ),
     .cfg_device_number( cfg_device_number ),
     .cfg_function_number( cfg_function_number ),
     .cfg_dstatus( cfg_dstatus ),
     .cfg_dcommand( cfg_dcommand ),
     .cfg_lcommand( cfg_lcommand ),
     .cfg_dsn(64'd0),

     .cfg_pm_halt_aspm_l0s(1'b0),
     .cfg_pm_halt_aspm_l1(1'b0),
     .cfg_pm_force_state_en(1'b0),
     .cfg_pm_force_state( 2'b00 ),
     .cfg_pciecap_interrupt_msgnum( 5'd0 ), // Only one MSI anyhow
     .cfg_interrupt_stat( 1'b0 ), // Never set the Interrupt Status bit

     .cfg_aer_interrupt_msgnum( 5'd0 ), // Only root ports set this
      
     .cfg_turnoff_ok( 1'b0 ),

     .pl_directed_link_auton( 1'b0 ),
     .pl_directed_link_change( 2'b00 ), // Don't change link parameters
     .pl_directed_link_speed( 1'b1 ), // Ignored by PCIe core
     .pl_directed_link_width( 2'b11 ),  // Ignored by PCIe core
     .pl_upstream_prefer_deemph( 1'b0 ), // Ignored by PCIe core

     .PIPE_PCLK_IN                              ( PIPE_PCLK_IN ),
     .PIPE_RXUSRCLK_IN                          ( PIPE_RXUSRCLK_IN ),
     .PIPE_RXOUTCLK_IN                          ( PIPE_RXOUTCLK_IN ),
     .PIPE_DCLK_IN                              ( PIPE_DCLK_IN ),
     .PIPE_USERCLK1_IN                          ( PIPE_USERCLK1_IN ),
     .PIPE_OOBCLK_IN                            ( PIPE_OOBCLK_IN ),
     .PIPE_USERCLK2_IN                          ( PIPE_USERCLK2_IN ),
     .PIPE_MMCM_LOCK_IN                         ( PIPE_MMCM_LOCK_IN ),
     
     .PIPE_TXOUTCLK_OUT                         ( PIPE_TXOUTCLK_OUT ),
     .PIPE_RXOUTCLK_OUT                         ( PIPE_RXOUTCLK_OUT ),
     .PIPE_PCLK_SEL_OUT                         ( PIPE_PCLK_SEL_OUT ),
     .PIPE_GEN3_OUT                             ( PIPE_GEN3_OUT ),
     
     .sys_clk(pcie_ref_clk),
     .sys_reset( !PCIE_PERST_B_LS )
      );

   pcie_k7_8x_pipe_clock #
     (
      .PCIE_ASYNC_EN                  ( "FALSE" ),     // PCIe async enable
      .PCIE_TXBUF_EN                  ( "FALSE" ),     // PCIe TX buffer enable for Gen1/Gen2 only
      .PCIE_LANE                      ( 6'h08 ),     // PCIe number of lanes
      .PCIE_LINK_SPEED              ( 3 ),
      .PCIE_REFCLK_FREQ               ( 0 ),     // PCIe reference clock frequency
      .PCIE_USERCLK1_FREQ             ( 4 ),     // PCIe user clock 1 frequency
      .PCIE_USERCLK2_FREQ             ( 4 ),     // PCIe user clock 2 frequency
      .PCIE_DEBUG_MODE                ( 0 )
      )
     pipe_clock
       (
	
        //---------- Input -------------------------------------
        .CLK_CLK                        ( pcie_ref_clk ),
        .CLK_TXOUTCLK                   ( PIPE_TXOUTCLK_OUT ),     // Reference clock from lane 0
        .CLK_RXOUTCLK_IN                ( PIPE_RXOUTCLK_OUT ),
        .CLK_RST_N                      ( 1'b1 ),
        .CLK_PCLK_SEL                   ( PIPE_PCLK_SEL_OUT ),
        .CLK_GEN3                       ( PIPE_GEN3_OUT ),
	
        //---------- Output ------------------------------------
        .CLK_PCLK                       ( PIPE_PCLK_IN ),
        .CLK_RXUSRCLK                   ( PIPE_RXUSRCLK_IN ),
        .CLK_RXOUTCLK_OUT               ( PIPE_RXOUTCLK_IN ),
        .CLK_DCLK                       ( PIPE_DCLK_IN ),
        .CLK_OOBCLK                     ( PIPE_OOBCLK_IN ),
        .CLK_USERCLK1                   ( PIPE_USERCLK1_IN ),
        .CLK_USERCLK2                   ( PIPE_USERCLK2_IN ),
        .CLK_MMCM_LOCK                  ( PIPE_MMCM_LOCK_IN )
	);

  xillybus_core  xillybus_core_ins(.trn_reset_n_w(trn_reset_n),
    .user_w_auxcmd3_membank_16_wren_w(user_w_auxcmd3_membank_16_wren),
    .user_w_auxcmd3_membank_16_data_w(user_w_auxcmd3_membank_16_data),
    .user_w_auxcmd3_membank_16_full_w(user_w_auxcmd3_membank_16_full),
    .user_w_auxcmd3_membank_16_open_w(user_w_auxcmd3_membank_16_open),
    .user_auxcmd3_membank_16_addr_w(user_auxcmd3_membank_16_addr),
    .user_auxcmd3_membank_16_addr_update_w(user_auxcmd3_membank_16_addr_update),
    .trn_lnk_up_n_w(trn_lnk_up_n), .quiesce_w(quiesce), .GPIO_LED_w(GPIO_LED),
    .s_axis_tx_tready_w(s_axis_tx_tready), .s_axis_tx_tdata_w(s_axis_tx_tdata),
    .s_axis_tx_tkeep_w(s_axis_tx_tkeep), .s_axis_tx_tlast_w(s_axis_tx_tlast),
    .s_axis_tx_tvalid_w(s_axis_tx_tvalid), .m_axis_rx_tdata_w(m_axis_rx_tdata),
    .m_axis_rx_tkeep_w(m_axis_rx_tkeep), .m_axis_rx_tlast_w(m_axis_rx_tlast),
    .m_axis_rx_tvalid_w(m_axis_rx_tvalid), .m_axis_rx_tready_w(m_axis_rx_tready),
    .cfg_interrupt_n_w(cfg_interrupt_n), .cfg_interrupt_rdy_n_w(cfg_interrupt_rdy_n),
    .cfg_bus_number_w(cfg_bus_number), .cfg_device_number_w(cfg_device_number),
    .cfg_function_number_w(cfg_function_number), .cfg_dcommand_w(cfg_dcommand),
    .cfg_lcommand_w(cfg_lcommand), .cfg_dstatus_w(cfg_dstatus),
    .trn_rerrfwd_n_w(trn_rerrfwd_n), .trn_fc_cplh_w(trn_fc_cplh),
    .trn_fc_cpld_w(trn_fc_cpld), .trn_terr_drop_n_w(trn_terr_drop_n),
    .user_r_neural_data_32_rden_w(user_r_neural_data_32_rden),
    .user_r_neural_data_32_data_w(user_r_neural_data_32_data),
    .user_r_neural_data_32_empty_w(user_r_neural_data_32_empty),
    .user_r_neural_data_32_eof_w(user_r_neural_data_32_eof),
    .user_r_neural_data_32_open_w(user_r_neural_data_32_open),
    .user_r_control_regs_16_rden_w(user_r_control_regs_16_rden),
    .user_r_control_regs_16_data_w(user_r_control_regs_16_data),
    .user_r_control_regs_16_empty_w(user_r_control_regs_16_empty),
    .user_r_control_regs_16_eof_w(user_r_control_regs_16_eof),
    .user_r_control_regs_16_open_w(user_r_control_regs_16_open),
    .user_w_control_regs_16_wren_w(user_w_control_regs_16_wren),
    .user_w_control_regs_16_data_w(user_w_control_regs_16_data),
    .user_w_control_regs_16_full_w(user_w_control_regs_16_full),
    .user_w_control_regs_16_open_w(user_w_control_regs_16_open),
    .user_control_regs_16_addr_w(user_control_regs_16_addr),
    .user_control_regs_16_addr_update_w(user_control_regs_16_addr_update),
    .user_r_status_regs_16_rden_w(user_r_status_regs_16_rden),
    .user_r_status_regs_16_data_w(user_r_status_regs_16_data),
    .user_r_status_regs_16_empty_w(user_r_status_regs_16_empty),
    .user_r_status_regs_16_eof_w(user_r_status_regs_16_eof),
    .user_r_status_regs_16_open_w(user_r_status_regs_16_open),
    .user_status_regs_16_addr_w(user_status_regs_16_addr),
    .user_status_regs_16_addr_update_w(user_status_regs_16_addr_update),
    .user_w_auxcmd1_membank_16_wren_w(user_w_auxcmd1_membank_16_wren),
    .user_w_auxcmd1_membank_16_data_w(user_w_auxcmd1_membank_16_data),
    .user_w_auxcmd1_membank_16_full_w(user_w_auxcmd1_membank_16_full),
    .user_w_auxcmd1_membank_16_open_w(user_w_auxcmd1_membank_16_open),
    .bus_clk_w(bus_clk), .user_auxcmd1_membank_16_addr_w(user_auxcmd1_membank_16_addr),
    .user_auxcmd1_membank_16_addr_update_w(user_auxcmd1_membank_16_addr_update),
    .user_w_auxcmd2_membank_16_wren_w(user_w_auxcmd2_membank_16_wren),
    .user_w_auxcmd2_membank_16_data_w(user_w_auxcmd2_membank_16_data),
    .user_w_auxcmd2_membank_16_full_w(user_w_auxcmd2_membank_16_full),
    .user_w_auxcmd2_membank_16_open_w(user_w_auxcmd2_membank_16_open),
    .user_auxcmd2_membank_16_addr_w(user_auxcmd2_membank_16_addr),
    .user_auxcmd2_membank_16_addr_update_w(user_auxcmd2_membank_16_addr_update));

endmodule
