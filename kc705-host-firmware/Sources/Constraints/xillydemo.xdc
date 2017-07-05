create_clock -period 10.000 -name sys_clk [get_pins -match_style ucf */pcieclk_ibuf/O]
create_clock -period 5.000 -name board_clk [get_ports SYSCLK_P]
create_generated_clock -name dataclk -source [get_ports SYSCLK_P] -divide_by 100 -multiply_by 42 [get_nets dataclk]
create_clock -period 20.000 -name gpclk [get_ports pclk_P]

#create_generated_clock -name clk4Hz -source [get_ports SYSCLK_P] -divide_by 50000000 [get_nets clk4Hz]

#create_generated_clock -name clk50M -source [get_ports pclk_P] -divide_by 1 [get_nets clk50M]

#set_clock_groups -name async_pcie_data -asynchronous -group {board_clk dataclk clkfbout clk_buf_out dataclk clk4Hz} -group {sys_clk userclk1} -group {gpclk, clk_out1_clk_wiz_1}
set_clock_groups -name async_pcie_data -asynchronous -group {board_clk dataclk clkfbout clk_buf_out dataclk} -group {sys_clk userclk1} -group {gpclk, clk_out1_clk_wiz_1}

set_false_path -from [get_pins clkgen/start_cd/toggleA*/*] -to [get_pins clkgen/start_cd/syncA*/*]
set_false_path -from [get_pins xillybus_ins/xillybus_core_ins/unitr_2_ins/user_w_control_regs_16_open/C] -to [get_pins -match_style ucf clkgen/*]

set_multicycle_path -hold -end -from [get_pins dataclk_O*/*] -to [get_pins clkgen/pll_O*/*] 1
set_multicycle_path -hold -end -from [get_pins dataclk_M*/*] -to [get_pins clkgen/pll_M*/*] 1
set_multicycle_path -hold -end -from [get_pins dataclk_D*/*] -to [get_pins clkgen/pll_D*/*] 1

set_false_path -to [get_pins -match_style ucf */pipe_clock/pclk_i1_bufgctrl.pclk_i1/S0]
set_false_path -to [get_pins -match_style ucf */pipe_clock/pclk_i1_bufgctrl.pclk_i1/S1]
set_case_analysis 1 [get_pins -match_style ucf */pipe_clock/pclk_i1_bufgctrl.pclk_i1/S0]
set_case_analysis 0 [get_pins -match_style ucf */pipe_clock/pclk_i1_bufgctrl.pclk_i1/S1]

set_property DONT_TOUCH true [get_cells -of [get_nets -of [get_pins -match_style ucf */pipe_clock/pclk_i1_bufgctrl.pclk_i1/S0]]]
set_property DONT_TOUCH true [get_cells -hier -filter name=~*/pipe_clock/pclk_sel*]

set_false_path -from [get_ports PCIE_PERST_B_LS]

set_property LOC IBUFDS_GTE2_X0Y1 [get_cells -match_style ucf */pcieclk_ibuf]

set_property PACKAGE_PIN G25 [get_ports PCIE_PERST_B_LS]
set_property IOSTANDARD LVCMOS25 [get_ports PCIE_PERST_B_LS]
set_property PULLUP true [get_ports PCIE_PERST_B_LS]

set_property -dict {PACKAGE_PIN AB8 IOSTANDARD LVCMOS15} [get_ports {GPIO_LED[0]}]
set_property -dict {PACKAGE_PIN AA8 IOSTANDARD LVCMOS15} [get_ports {GPIO_LED[1]}]
set_property -dict {PACKAGE_PIN AC9 IOSTANDARD LVCMOS15} [get_ports {GPIO_LED[2]}]
set_property -dict {PACKAGE_PIN AB9 IOSTANDARD LVCMOS15} [get_ports {GPIO_LED[3]}]

set_property PACKAGE_PIN F16 [get_ports RESET_LED]
set_property IOSTANDARD LVCMOS25 [get_ports RESET_LED]

set_property PACKAGE_PIN E18 [get_ports SPI_LED]
set_property IOSTANDARD LVCMOS25 [get_ports SPI_LED]

set_property PACKAGE_PIN G19 [get_ports OVERFLOW_LED]
set_property IOSTANDARD LVCMOS25 [get_ports OVERFLOW_LED]

set_property IOSTANDARD LVDS [get_ports SYSCLK_P]
set_property PACKAGE_PIN AD11 [get_ports SYSCLK_N]
set_property PACKAGE_PIN AD12 [get_ports SYSCLK_P]
set_property IOSTANDARD LVDS [get_ports SYSCLK_N]

# isolated output from FPGA
set_property PACKAGE_PIN AK26 [get_ports sma_out_isol_H23]
set_property IOSTANDARD LVCMOS25 [get_ports sma_out_isol_H23]

#GPIO debug #J13
set_property PACKAGE_PIN Y23 [get_ports ledinput] 
set_property IOSTANDARD LVCMOS25 [get_ports ledinput]
#J14
set_property PACKAGE_PIN Y24 [get_ports ledoutput]
set_property IOSTANDARD LVCMOS25 [get_ports ledoutput]

# isolated inputs to FPGA


# direct SMAs (G24, 25, 27)
#set_property PACKAGE_PIN AJ27 [get_ports sma_direct_G24]
#set_property IOSTANDARD LVCMOS25 [get_ports sma_direct_G24]

#set_property PACKAGE_PIN AK28 [get_ports sma_direct_G25]
#set_property IOSTANDARD LVCMOS25 [get_ports sma_direct_G25]

#set_property PACKAGE_PIN AC26 [get_ports sma_direct_G27]
#set_property IOSTANDARD LVCMOS25 [get_ports sma_direct_G27]


# C spi port (isolated, simple cmos33 logic, gets translated to lvds after isolator)

#set_property PACKAGE_PIN AE25 [get_ports MISO_C1_PORT]
#set_property IOSTANDARD LVCMOS33 [get_ports MISO_C1_PORT]

#set_property PACKAGE_PIN AF25 [get_ports MISO_C2_PORT]
#set_property IOSTANDARD LVCMOS33 [get_ports MISO_C2_PORT]

#set_property PACKAGE_PIN AC24 [get_ports MOSI_C_PORT]
#set_property IOSTANDARD LVCMOS33 [get_ports MOSI_C_PORT]

#set_property PACKAGE_PIN AD24 [get_ports SCLK_C_PORT]
#set_property IOSTANDARD LVCMOS33 [get_ports SCLK_C_PORT]

#set_property PACKAGE_PIN AJ26 [get_ports CS_C_PORT]
#set_property IOSTANDARD LVCMOS33 [get_ports CS_C_PORT]

#pclk -----------------------------------------------
set_property PACKAGE_PIN AG29 [get_ports pclk_P]
set_property IOSTANDARD LVDS_25 [get_ports pclk_P]
set_property DIFF_TERM TRUE [get_ports pclk_P]


set_property PACKAGE_PIN AH29 [get_ports pclk_N]
set_property IOSTANDARD LVDS_25 [get_ports pclk_N]
set_property DIFF_TERM TRUE [get_ports pclk_N]

#LED GPO -----------------------------------------------
set_property PACKAGE_PIN AJ27 [get_ports GPO_LED_P]
set_property IOSTANDARD LVDS_25 [get_ports GPO_LED_P]
#set_property DIFF_TERM TRUE [get_ports GPO_LED_P]

set_property PACKAGE_PIN AK28 [get_ports GPO_LED_N]
set_property IOSTANDARD LVDS_25 [get_ports GPO_LED_N]
#set_property DIFF_TERM TRUE [get_ports GPO_LED_N]

#D11 -----------------------------------------------
set_property PACKAGE_PIN AF20 [get_ports D11_P]
set_property IOSTANDARD LVDS_25 [get_ports D11_P]
set_property DIFF_TERM TRUE [get_ports D11_P]

set_property PACKAGE_PIN AF21 [get_ports D11_N]
set_property IOSTANDARD LVDS_25 [get_ports D11_N]
set_property DIFF_TERM TRUE [get_ports D11_N]

#D10 -----------------------------------------------
set_property PACKAGE_PIN AG20 [get_ports D10_P]
set_property IOSTANDARD LVDS_25 [get_ports D10_P]
set_property DIFF_TERM TRUE [get_ports D10_P]

set_property PACKAGE_PIN AH20 [get_ports D10_N]
set_property IOSTANDARD LVDS_25 [get_ports D10_N]
set_property DIFF_TERM TRUE [get_ports D10_N]

#D9 -----------------------------------------------
set_property PACKAGE_PIN AK20 [get_ports D9_P]
set_property IOSTANDARD LVDS_25 [get_ports D9_P]
set_property DIFF_TERM TRUE [get_ports D9_P]

set_property PACKAGE_PIN AK21 [get_ports D9_N]
set_property IOSTANDARD LVDS_25 [get_ports D9_N]
set_property DIFF_TERM TRUE [get_ports D9_N]

#D8 -----------------------------------------------
set_property PACKAGE_PIN AH21 [get_ports D8_P]
set_property IOSTANDARD LVDS_25 [get_ports D8_P]
set_property DIFF_TERM TRUE [get_ports D8_P]

set_property PACKAGE_PIN AJ21 [get_ports D8_N]
set_property IOSTANDARD LVDS_25 [get_ports D8_N]
set_property DIFF_TERM TRUE [get_ports D8_P]

#D7 -----------------------------------------------
set_property PACKAGE_PIN AG22 [get_ports D7_P]
set_property IOSTANDARD LVDS_25 [get_ports D7_P]
set_property DIFF_TERM TRUE [get_ports D7_P]

set_property PACKAGE_PIN AH22 [get_ports D7_N]
set_property IOSTANDARD LVDS_25 [get_ports D7_N]
set_property DIFF_TERM TRUE [get_ports D7_N]

#D6 -----------------------------------------------
set_property PACKAGE_PIN AJ22 [get_ports D6_P]
set_property IOSTANDARD LVDS_25 [get_ports D6_P]
set_property DIFF_TERM TRUE [get_ports D6_P]

set_property PACKAGE_PIN AJ23 [get_ports D6_N]
set_property IOSTANDARD LVDS_25 [get_ports D6_N]
set_property DIFF_TERM TRUE [get_ports D6_N]

#D5 -----------------------------------------------
set_property PACKAGE_PIN AG25 [get_ports D5_P]
set_property IOSTANDARD LVDS_25 [get_ports D5_P]
set_property DIFF_TERM TRUE [get_ports D5_P]

set_property PACKAGE_PIN AH25 [get_ports D5_N]
set_property IOSTANDARD LVDS_25 [get_ports D5_N]
set_property DIFF_TERM TRUE [get_ports D5_N]

#D4 -----------------------------------------------
set_property PACKAGE_PIN AK23 [get_ports D4_P]
set_property IOSTANDARD LVDS_25 [get_ports D4_P]
set_property DIFF_TERM TRUE [get_ports D4_P]

set_property PACKAGE_PIN AK24 [get_ports D4_N]
set_property IOSTANDARD LVDS_25 [get_ports D4_N]
set_property DIFF_TERM TRUE [get_ports D4_N]

#D3 -----------------------------------------------
set_property PACKAGE_PIN AJ24 [get_ports D3_P]
set_property IOSTANDARD LVDS_25 [get_ports D3_P]
set_property DIFF_TERM TRUE [get_ports D3_P]

set_property PACKAGE_PIN AK25 [get_ports D3_N]
set_property IOSTANDARD LVDS_25 [get_ports D3_N]
set_property DIFF_TERM TRUE [get_ports D3_N]

#D2 -----------------------------------------------
set_property PACKAGE_PIN AA20 [get_ports D2_P]
set_property IOSTANDARD LVDS_25 [get_ports D2_P]
set_property DIFF_TERM TRUE [get_ports D2_P]

set_property PACKAGE_PIN AB20 [get_ports D2_N]
set_property IOSTANDARD LVDS_25 [get_ports D2_N]
set_property DIFF_TERM TRUE [get_ports D2_N]

#D1 -----------------------------------------------
#set_property PACKAGE_PIN AE25 [get_ports D1_P]
#set_property IOSTANDARD LVDS_25 [get_ports D1_P]

#set_property PACKAGE_PIN AF25 [get_ports D1_N]
#set_property IOSTANDARD LVDS_25 [get_ports D1_N]
#D0 -----------------------------------------------
#this is used as hsync 
set_property PACKAGE_PIN AB24 [get_ports hsync_P]
set_property IOSTANDARD LVDS_25 [get_ports hsync_P]
set_property DIFF_TERM TRUE [get_ports hsync_P]

set_property PACKAGE_PIN AC25 [get_ports hsync_N]
set_property IOSTANDARD LVDS_25 [get_ports hsync_N]
set_property DIFF_TERM TRUE [get_ports hsync_N]

#VSYNC---------------------------------------------
set_property PACKAGE_PIN AF22 [get_ports vsync_P]
set_property IOSTANDARD LVDS_25 [get_ports vsync_P]
set_property DIFF_TERM TRUE [get_ports vsync_P]

set_property PACKAGE_PIN AG23 [get_ports vsync_N]
set_property IOSTANDARD LVDS_25 [get_ports vsync_N]
set_property DIFF_TERM TRUE [get_ports vsync_N]

#AUX_VIDEO_IN
#set_property PACKAGE_PIN AC25 [get_ports aux_vid_P]
#set_property IOSTANDARD LVDS_25 [get_ports aux_vid_P]
#set_property DIFF_TERM TRUE [get_ports aux_vid_P]

#set_property PACKAGE_PIN AD26 [get_ports aux_vid_N]
#set_property IOSTANDARD LVDS_25 [get_ports aux_vid_N]
#set_property DIFF_TERM TRUE [get_ports aux_vid_N]


#AUX termination 
set_property PACKAGE_PIN AD21 [get_ports dif_term_0_P]
set_property IOSTANDARD LVDS_25 [get_ports dif_term_0_P]
set_property DIFF_TERM TRUE [get_ports dif_term_0_P]

set_property PACKAGE_PIN AE21 [get_ports dif_term_0_N]
set_property IOSTANDARD LVDS_25 [get_ports dif_term_0_N]
set_property DIFF_TERM TRUE [get_ports dif_term_0_N]

set_property PACKAGE_PIN AC22 [get_ports dif_term_1_P]
set_property IOSTANDARD LVDS_25 [get_ports dif_term_1_P]
set_property DIFF_TERM TRUE [get_ports dif_term_1_P]

set_property PACKAGE_PIN AD22 [get_ports dif_term_1_N]
set_property IOSTANDARD LVDS_25 [get_ports dif_term_1_N]
set_property DIFF_TERM TRUE [get_ports dif_term_1_N]


#HSYNC---------------------------------------------
#set_property PACKAGE_PIN N8 [get_ports hsync_P]
#set_property IOSTANDARD LVDS_25 [get_ports hsync_P]
#set_property DIFF_TERM TRUE [get_ports hsync_P]

#set_property PACKAGE_PIN N7 [get_ports hsync_N]
#set_property IOSTANDARD LVDS_25 [get_ports hsync_N]
#set_property DIFF_TERM TRUE [get_ports hsync_N]

# TODO: If we want to acctually use the isolated SPI busses, we need to make sure they use
# the same VCCO as the singled ended ports on the same bank. If they need to use a different
# VCCO, then their DIFF_TERM property should be set to false. Ideally, we should not have any
# non isolated pins anyway, so this is probably all mute.

## B spi port (non-isolated, goes straight to FMC pins)
#set_property PACKAGE_PIN AG30 [get_ports MISO_B1_p]
#set_property PACKAGE_PIN AH30 [get_ports MISO_B1_n]
#set_property IOSTANDARD LVDS_25 [get_ports {MISO_B1_p MISO_B1_n}]

#set_property PACKAGE_PIN AG27 [get_ports MISO_B2_p]
#set_property PACKAGE_PIN AG28 [get_ports MISO_B2_n]
#set_property IOSTANDARD LVDS_25 [get_ports {MISO_B2_p MISO_B2_n}]

#set_property PACKAGE_PIN AE30 [get_ports MOSI_B_p]
#set_property PACKAGE_PIN AF30 [get_ports MOSI_B_n]
#set_property IOSTANDARD LVDS_25 [get_ports {MOSI_B_p MOSI_B_n}]

#set_property PACKAGE_PIN AB29 [get_ports SCLK_B_p]
#set_property PACKAGE_PIN AB30 [get_ports SCLK_B_n]
#set_property IOSTANDARD LVDS_25 [get_ports {SCLK_B_p SCLK_B_n}]

#set_property PACKAGE_PIN Y30 [get_ports CS_B_p]
#set_property PACKAGE_PIN AA30 [get_ports CS_B_n]
#set_property IOSTANDARD LVDS_25 [get_ports {CS_B_p CS_B_n}]


set_property CONFIG_MODE BPI16 [current_design]
