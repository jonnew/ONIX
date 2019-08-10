####################################################################################################################
#                                               CLOCK 100MHz                                                       #
####################################################################################################################
set_property -dict { PACKAGE_PIN "F22"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { CLK1 }]     ;                           # IO_L12P_T1_MRCC_14            Sch = CLK1

set_property -dict { PACKAGE_PIN "D23"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { CLK_DIR }]  ;                           # IO_L11P_T1_SRCC_14            Sch = CLK_DIR

####################################################################################################################
#                                              150MHz LVDS Oscillator                                              #
####################################################################################################################
set_property -dict { PACKAGE_PIN "G24"    IOSTANDARD TMDS_33   SLEW FAST} [get_ports { CLK2_P }]     ;                          # IO_L14P_T2_SRCC_14            Sch = CLK2_P
set_property -dict { PACKAGE_PIN "F24"    IOSTANDARD TMDS_33   SLEW FAST} [get_ports { CLK2_N }]     ;                          # IO_L14N_T2_SRCC_14            Sch = CLK2_N

####################################################################################################################
#                                       RESET - SW2 & FAN PWM                                                      #
####################################################################################################################
set_property -dict { PACKAGE_PIN "C26"    IOSTANDARD LVCMOS33   SLEW FAST   PULLDOWN True} [get_ports { RESET }]    ;           # IO_L5N_T0_D07_14              Sch = RESET
set_property -dict { PACKAGE_PIN "J25"    IOSTANDARD LVCMOS33   SLEW FAST   PULLDOWN True} [get_ports { FAN_PWM }]  ;           # IO_L22N_T3_A04_D20_14         Sch = FAN_PWM

####################################################################################################################
#                                               ADC                                                                #
####################################################################################################################
set_property -dict { PACKAGE_PIN "B16"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { ADC_A_N[0] }]   ;                       # IO_L1N_T0_AD0N_15             Sch = ADC_A0_N
set_property -dict { PACKAGE_PIN "C16"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { ADC_A_P[0] }]   ;                       # IO_L1P_T0_AD0P_15             Sch = ADC_A0_P
set_property -dict { PACKAGE_PIN "A19"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { ADC_A_N[1] }]   ;                       # IO_L2N_T0_AD8N_15             Sch = ADC_A1_N
set_property -dict { PACKAGE_PIN "A18"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { ADC_A_P[1] }]   ;                       # IO_L2P_T0_AD8P_15             Sch = ADC_A1_P
set_property -dict { PACKAGE_PIN "A17"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { ADC_A_N[2] }]   ;                       # IO_L3N_T0_DQS_AD1N_15         Sch = ADC_A2_N
set_property -dict { PACKAGE_PIN "B17"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { ADC_A_P[2] }]   ;                       # IO_L3P_T0_DQS_AD1P_15         Sch = ADC_A2_P
set_property -dict { PACKAGE_PIN "N11"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { ADC_VREF_N }]   ;                       # VREFN_0                       Sch = ADC_VREF_N
set_property -dict { PACKAGE_PIN "P12"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { ADC_VREF_P }]   ;                       # VREFP_0                       Sch = ADC_VREF_P

####################################################################################################################
#                                              FT234 Signals                                                       #
####################################################################################################################
set_property -dict { PACKAGE_PIN "K22"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { FT234_TXD }]    ;                       # IO_L23N_T3_A02_D18_14         Sch = FT234_TXD
set_property -dict { PACKAGE_PIN "L22"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { FT234_RTS }]    ;                       # IO_L23P_T3_A03_D19_14         Sch = FT234_RTS
set_property -dict { PACKAGE_PIN "L23"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { FT234_CTS }]    ;                       # IO_25_14                      Sch = FT234_CTS
set_property -dict { PACKAGE_PIN "H22"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { FT234_RXD }]    ;                       # IO_L21N_T3_DQS_A06_D22_14     Sch = FT234_RXD
set_property -dict { PACKAGE_PIN "K23"    IOSTANDARD LVCMOS33   SLEW FAST} [get_ports { FT234_CBUS0 }]  ;                       # IO_L24P_T3_A01_D17_14         Sch = FT234_CBUS0

####################################################################################################################
#                                              DDR3 SODIMM                                                         #
#                                              Data Width : 64                                                     #
####################################################################################################################
set_property -dict { PACKAGE_PIN "V19"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[0] }]       ;       # IO_L23N_T3_32                 Sch = DDR3-DQ0
set_property -dict { PACKAGE_PIN "V16"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[1] }]       ;       # IO_L20P_T3_32                 Sch = DDR3-DQ1
set_property -dict { PACKAGE_PIN "Y17"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[2] }]       ;       # IO_L19P_T3_32                 Sch = DDR3-DQ2
set_property -dict { PACKAGE_PIN "V14"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[3] }]       ;       # IO_L24P_T3_32                 Sch = DDR3-DQ3
set_property -dict { PACKAGE_PIN "V17"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[4] }]       ;       # IO_L20N_T3_32                 Sch = DDR3-DQ4
set_property -dict { PACKAGE_PIN "V18"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[5] }]       ;       # IO_L23P_T3_32                 Sch = DDR3-DQ5
set_property -dict { PACKAGE_PIN "W14"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[6] }]       ;       # IO_L24N_T3_32                 Sch = DDR3-DQ6
set_property -dict { PACKAGE_PIN "W15"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[7] }]       ;       # IO_L22P_T3_32                 Sch = DDR3-DQ7
set_property -dict { PACKAGE_PIN "AB17"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[8] }]       ;       # IO_L14P_T2_SRCC_32            Sch = DDR3-DQ8
set_property -dict { PACKAGE_PIN "AB19"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[9] }]       ;       # IO_L18P_T2_32                 Sch = DDR3-DQ9
set_property -dict { PACKAGE_PIN "AC18"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[10] }]      ;       # IO_L13P_T2_MRCC_32            Sch = DDR3-DQ10
set_property -dict { PACKAGE_PIN "AC19"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[11] }]      ;       # IO_L17P_T2_32                 Sch = DDR3-DQ11
set_property -dict { PACKAGE_PIN "AA19"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[12] }]      ;       # IO_L16P_T2_32                 Sch = DDR3-DQ12
set_property -dict { PACKAGE_PIN "AA20"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[13] }]      ;       # IO_L16N_T2_32                 Sch = DDR3-DQ13
set_property -dict { PACKAGE_PIN "AC17"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[14] }]      ;       # IO_L14N_T2_SRCC_32            Sch = DDR3-DQ14
set_property -dict { PACKAGE_PIN "AD19"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[15] }]      ;       # IO_L17N_T2_32                 Sch = DDR3-DQ15
set_property -dict { PACKAGE_PIN "AD16"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[16] }]      ;       # IO_L6P_T0_32                  Sch = DDR3-DQ16
set_property -dict { PACKAGE_PIN "AD15"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[17] }]      ;       # IO_L4P_T0_32                  Sch = DDR3-DQ17
set_property -dict { PACKAGE_PIN "AF20"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[18] }]      ;       # IO_L5N_T0_32                  Sch = DDR3-DQ18
set_property -dict { PACKAGE_PIN "AE17"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[19] }]      ;       # IO_L1P_T0_32                  Sch = DDR3-DQ19
set_property -dict { PACKAGE_PIN "AF17"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[20] }]      ;       # IO_L1N_T0_32                  Sch = DDR3-DQ20
set_property -dict { PACKAGE_PIN "AF19"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[21] }]      ;       # IO_L5P_T0_32                  Sch = DDR3-DQ21
set_property -dict { PACKAGE_PIN "AF14"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[22] }]      ;       # IO_L2P_T0_32                  Sch = DDR3-DQ22
set_property -dict { PACKAGE_PIN "AF15"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[23] }]      ;       # IO_L2N_T0_32                  Sch = DDR3-DQ23
set_property -dict { PACKAGE_PIN "AB16"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[24] }]      ;       # IO_L12P_T1_MRCC_32            Sch = DDR3-DQ24
set_property -dict { PACKAGE_PIN "AA15"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[25] }]      ;       # IO_L7N_T1_32                  Sch = DDR3-DQ25
set_property -dict { PACKAGE_PIN "AA14"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[26] }]      ;       # IO_L7P_T1_32                  Sch = DDR3-DQ26
set_property -dict { PACKAGE_PIN "AC14"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[27] }]      ;       # IO_L8P_T1_32                  Sch = DDR3-DQ27
set_property -dict { PACKAGE_PIN "AA18"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[28] }]      ;       # IO_L11N_T1_SRCC_32            Sch = DDR3-DQ28
set_property -dict { PACKAGE_PIN "AA17"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[29] }]      ;       # IO_L11P_T1_SRCC_32            Sch = DDR3-DQ29
set_property -dict { PACKAGE_PIN "AD14"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[30] }]      ;       # IO_L8N_T1_32                  Sch = DDR3-DQ30
set_property -dict { PACKAGE_PIN "AB14"   IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[31] }]      ;       # IO_L10P_T1_32                 Sch = DDR3-DQ31
set_property -dict { PACKAGE_PIN "AE3"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[32] }]      ;       # IO_L22P_T3_34                 Sch = DDR3-DQ32
set_property -dict { PACKAGE_PIN "AE6"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[33] }]      ;       # IO_L23P_T3_34                 Sch = DDR3-DQ33
set_property -dict { PACKAGE_PIN "AE2"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[34] }]      ;       # IO_L22N_T3_34                 Sch = DDR3-DQ34
set_property -dict { PACKAGE_PIN "AF3"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[35] }]      ;       # IO_L24P_T3_34                 Sch = DDR3-DQ35
set_property -dict { PACKAGE_PIN "AD4"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[36] }]      ;       # IO_L19P_T3_34                 Sch = DDR3-DQ36
set_property -dict { PACKAGE_PIN "AE5"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[37] }]      ;       # IO_L23N_T3_34                 Sch = DDR3-DQ37
set_property -dict { PACKAGE_PIN "AE1"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[38] }]      ;       # IO_L20N_T3_34                 Sch = DDR3-DQ38
set_property -dict { PACKAGE_PIN "AF2"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[39] }]      ;       # IO_L24N_T3_34                 Sch = DDR3-DQ39
set_property -dict { PACKAGE_PIN "AB6"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[40] }]      ;       # IO_L16P_T2_34                 Sch = DDR3-DQ40
set_property -dict { PACKAGE_PIN "Y6"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[41] }]      ;       # IO_L17P_T2_34                 Sch = DDR3-DQ41
set_property -dict { PACKAGE_PIN "AB4"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[42] }]      ;       # IO_L13N_T2_MRCC_34            Sch = DDR3-DQ42
set_property -dict { PACKAGE_PIN "AC4"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[43] }]      ;       # IO_L14P_T2_SRCC_34            Sch = DDR3-DQ43
set_property -dict { PACKAGE_PIN "AC6"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[44] }]      ;       # IO_L16N_T2_34                 Sch = DDR3-DQ44
set_property -dict { PACKAGE_PIN "AD6"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[45] }]      ;       # IO_L18P_T2_34                 Sch = DDR3-DQ45
set_property -dict { PACKAGE_PIN "Y5"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[46] }]      ;       # IO_L17N_T2_34                 Sch = DDR3-DQ46
set_property -dict { PACKAGE_PIN "AA4"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[47] }]      ;       # IO_L13P_T2_MRCC_34            Sch = DDR3-DQ47
set_property -dict { PACKAGE_PIN "AB2"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[48] }]      ;       # IO_L11P_T1_SRCC_34            Sch = DDR3-DQ48
set_property -dict { PACKAGE_PIN "AC2"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[49] }]      ;       # IO_L11N_T1_SRCC_34            Sch = DDR3-DQ49
set_property -dict { PACKAGE_PIN "V1"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[50] }]      ;       # IO_L8N_T1_34                  Sch = DDR3-DQ50
set_property -dict { PACKAGE_PIN "W1"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[51] }]      ;       # IO_L10P_T1_34                 Sch = DDR3-DQ51
set_property -dict { PACKAGE_PIN "V2"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[52] }]      ;       # IO_L8P_T1_34                  Sch = DDR3-DQ52
set_property -dict { PACKAGE_PIN "AA3"    IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[53] }]      ;       # IO_L12P_T1_MRCC_34            Sch = DDR3-DQ53
set_property -dict { PACKAGE_PIN "Y1"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[54] }]      ;       # IO_L10N_T1_34                 Sch = DDR3-DQ54
set_property -dict { PACKAGE_PIN "Y2"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[55] }]      ;       # IO_L7N_T1_34                  Sch = DDR3-DQ55
set_property -dict { PACKAGE_PIN "V4"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[56] }]      ;       # IO_L6P_T0_34                  Sch = DDR3-DQ56
set_property -dict { PACKAGE_PIN "V3"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[57] }]      ;       # IO_L4P_T0_34                  Sch = DDR3-DQ57
set_property -dict { PACKAGE_PIN "U2"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[58] }]      ;       # IO_L2P_T0_34                  Sch = DDR3-DQ58
set_property -dict { PACKAGE_PIN "U1"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[59] }]      ;       # IO_L2N_T0_34                  Sch = DDR3-DQ59
set_property -dict { PACKAGE_PIN "U7"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[60] }]      ;       # IO_L5P_T0_34                  Sch = DDR3-DQ60
set_property -dict { PACKAGE_PIN "W3"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[61] }]      ;       # IO_L4N_T0_34                  Sch = DDR3-DQ61
set_property -dict { PACKAGE_PIN "U6"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[62] }]      ;       # IO_L1P_T0_34                  Sch = DDR3-DQ62
set_property -dict { PACKAGE_PIN "U5"     IOSTANDARD SSTL135_T_DCI          SLEW FAST} [get_ports { ddr3_dq[63] }]      ;       # IO_L1N_T0_34                  Sch = DDR3-DQ63
set_property -dict { PACKAGE_PIN "W9"     IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[15] }]    ;       # IO_L3N_T0_DQS_33              Sch = DDR3-A15
set_property -dict { PACKAGE_PIN "Y10"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[14] }]    ;       # IO_L5N_T0_33                  Sch = DDR3-A14
set_property -dict { PACKAGE_PIN "Y8"     IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[13] }]    ;       # IO_L4P_T0_33                  Sch = DDR3-A13
set_property -dict { PACKAGE_PIN "W10"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[12] }]    ;       # IO_L3P_T0_DQS_33              Sch = DDR3-A12
set_property -dict { PACKAGE_PIN "Y7"     IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[11] }]    ;       # IO_L4N_T0_33                  Sch = DDR3-A11
set_property -dict { PACKAGE_PIN "Y11"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[10] }]    ;       # IO_L5P_T0_33                  Sch = DDR3-A10
set_property -dict { PACKAGE_PIN "V9"     IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[9] }]     ;       # IO_L6P_T0_33                  Sch = DDR3-A9
set_property -dict { PACKAGE_PIN "AD8"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[8] }]     ;       # IO_L9N_T1_DQS_33              Sch = DDR3-A8
set_property -dict { PACKAGE_PIN "AA9"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[7] }]     ;       # IO_L11P_T1_SRCC_33            Sch = DDR3-A7
set_property -dict { PACKAGE_PIN "AC9"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[6] }]     ;       # IO_L12P_T1_MRCC_33            Sch = DDR3-A6
set_property -dict { PACKAGE_PIN "AC8"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[5] }]     ;       # IO_L9P_T1_DQS_33              Sch = DDR3-A5
set_property -dict { PACKAGE_PIN "AA7"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[4] }]     ;       # IO_L8N_T1_33                  Sch = DDR3-A4
set_property -dict { PACKAGE_PIN "AB7"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[3] }]     ;       # IO_L10P_T1_33                 Sch = DDR3-A3
set_property -dict { PACKAGE_PIN "AC7"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[2] }]     ;       # IO_L10N_T1_33                 Sch = DDR3-A2
set_property -dict { PACKAGE_PIN "AE7"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[1] }]     ;       # IO_L7P_T1_33                  Sch = DDR3-A1
set_property -dict { PACKAGE_PIN "AF7"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_addr[0] }]     ;       # IO_L7N_T1_33                  Sch = DDR3-A0
set_property -dict { PACKAGE_PIN "AB9"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_ba[2] }]       ;       # IO_L11N_T1_SRCC_33            Sch = DDR3-BA2
set_property -dict { PACKAGE_PIN "AD9"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_ba[1] }]       ;       # IO_L12N_T1_MRCC_33            Sch = DDR3-BA1
set_property -dict { PACKAGE_PIN "AA8"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_ba[0] }]       ;       # IO_L8P_T1_33                  Sch = DDR3-BA0
set_property -dict { PACKAGE_PIN "AC13"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_ras_n }]       ;       # IO_L17P_T2_33                 Sch = DDR3-RAS#
set_property -dict { PACKAGE_PIN "AC12"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_cas_n }]       ;       # IO_L15N_T2_DQS_33             Sch = DDR3-CAS#
set_property -dict { PACKAGE_PIN "AA13"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_we_n  }]       ;       # IO_L16P_T2_33                 Sch = DDR3-WE#
set_property -dict { PACKAGE_PIN "AA2"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_reset_n }]     ;       # IO_L12N_T1_MRCC_34            Sch = DDR3-RESET#
set_property -dict { PACKAGE_PIN "AA10"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_cke[0] }]      ;       # IO_L14P_T2_SRCC_33            Sch = DDR3-CKE0
set_property -dict { PACKAGE_PIN "AB10"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_cke[1] }]      ;       # IO_L14N_T2_SRCC_33            Sch = DDR3-CKE1
set_property -dict { PACKAGE_PIN "AD13"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_odt[0] }]      ;       # IO_L17N_T2_33                 Sch = DDR3-ODT0
set_property -dict { PACKAGE_PIN "Y13"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_odt[1] }]      ;       # IO_L18P_T2_33                 Sch = DDR3-ODT1
set_property -dict { PACKAGE_PIN "AB12"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_cs_n[0] }]     ;       # IO_L15P_T2_DQS_33             Sch = DDR3-CS0#
set_property -dict { PACKAGE_PIN "AA12"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_cs_n[1] }]     ;       # IO_L16N_T2_33                 Sch = DDR3-CS1#
set_property -dict { PACKAGE_PIN "W16"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_dm[0] }]       ;       # IO_L22N_T3_32                 Sch = DDR3-DM0
set_property -dict { PACKAGE_PIN "AD18"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_dm[1] }]       ;       # IO_L13N_T2_MRCC_32            Sch = DDR3-DM1
set_property -dict { PACKAGE_PIN "AE15"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_dm[2] }]       ;       # IO_L4N_T0_32                  Sch = DDR3-DM2
set_property -dict { PACKAGE_PIN "AB15"   IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_dm[3] }]       ;       # IO_L10N_T1_32                 Sch = DDR3-DM3
set_property -dict { PACKAGE_PIN "AD1"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_dm[4] }]       ;       # IO_L20P_T3_34                 Sch = DDR3-DM4
set_property -dict { PACKAGE_PIN "AC3"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_dm[5] }]       ;       # IO_L14N_T2_SRCC_34            Sch = DDR3-DM5
set_property -dict { PACKAGE_PIN "Y3"     IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_dm[6] }]       ;       # IO_L7P_T1_34                  Sch = DDR3-DM6
set_property -dict { PACKAGE_PIN "V6"     IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_dm[7] }]       ;       # IO_L5N_T0_34                  Sch = DDR3-DM7
set_property -dict { PACKAGE_PIN "W18"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_p[0] }]    ;       # IO_L21P_T3_DQS_32             Sch = DDR3-DQS0_P
set_property -dict { PACKAGE_PIN "W19"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_n[0] }]    ;       # IO_L21N_T3_DQS_32             Sch = DDR3-DQS0_N
set_property -dict { PACKAGE_PIN "AD20"   IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_p[1] }]    ;       # IO_L15P_T2_DQS_32             Sch = DDR3-DQS1_P
set_property -dict { PACKAGE_PIN "AE20"   IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_n[1] }]    ;       # IO_L15N_T2_DQS_32             Sch = DDR3-DQS1_N
set_property -dict { PACKAGE_PIN "AE18"   IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_p[2] }]    ;       # IO_L3P_T0_DQS_32              Sch = DDR3-DQS2_P
set_property -dict { PACKAGE_PIN "AF18"   IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_n[2] }]    ;       # IO_L3N_T0_DQS_32              Sch = DDR3-DQS2_N
set_property -dict { PACKAGE_PIN "Y15"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_p[3] }]    ;       # IO_L9P_T1_DQS_32              Sch = DDR3-DQS3_P
set_property -dict { PACKAGE_PIN "Y16"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_n[3] }]    ;       # IO_L9N_T1_DQS_32              Sch = DDR3-DQS3_N
set_property -dict { PACKAGE_PIN "AF5"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_p[4] }]    ;       # IO_L21P_T3_DQS_34             Sch = DDR3-DQS4_P
set_property -dict { PACKAGE_PIN "AF4"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_n[4] }]    ;       # IO_L21N_T3_DQS_34             Sch = DDR3-DQS4_N
set_property -dict { PACKAGE_PIN "AA5"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_p[5] }]    ;       # IO_L15P_T2_DQS_34             Sch = DDR3-DQS5_P
set_property -dict { PACKAGE_PIN "AB5"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_n[5] }]    ;       # IO_L15N_T2_DQS_34             Sch = DDR3-DQS5_N
set_property -dict { PACKAGE_PIN "AB1"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_p[6]  }]   ;       # IO_L9P_T1_DQS_34              Sch = DDR3-DQS6_P
set_property -dict { PACKAGE_PIN "AC1"    IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_n[6] }]    ;       # IO_L9N_T1_DQS_34              Sch = DDR3-DQS6_N
set_property -dict { PACKAGE_PIN "W6"     IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_p[7] }]    ;       # IO_L3P_T0_DQS_34              Sch = DDR3-DQS7_P
set_property -dict { PACKAGE_PIN "W5"     IOSTANDARD DIFF_SSTL135_T_DCI     SLEW FAST} [get_ports { ddr3_dqs_n[7] }]    ;       # IO_L3N_T0_DQS_34              Sch = DDR3-DQS7_N
set_property -dict { PACKAGE_PIN "V11"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_ck_p[0] }]     ;       # IO_L1P_T0_33                  Sch = DDR3-CK0_P
set_property -dict { PACKAGE_PIN "W11"    IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_ck_n[0] }]     ;       # IO_L1N_T0_33                  Sch = DDR3-CK0_N
set_property -dict { PACKAGE_PIN "V8"     IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_ck_p[1] }]     ;       # IO_L2P_T0_33                  Sch = DDR3-CK1_P
set_property -dict { PACKAGE_PIN "V7"     IOSTANDARD SSTL135                SLEW FAST} [get_ports { ddr3_ck_n[1] }]     ;       # IO_L2N_T0_33                  Sch = DDR3-CK1_N


####################################################################################################################
#                                              QSPI - FLASH                                                        #
####################################################################################################################
set_property -dict { PACKAGE_PIN "C23"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SPI_CS_N }]     ;                   # IO_L6P_T0_FCS_B_14            Sch = SPI_CS_N
set_property -dict { PACKAGE_PIN "B24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SPI_DQ[0] }]    ;                   # IO_L1P_T0_D00_MOSI_14         Sch = SPI_DQ0
set_property -dict { PACKAGE_PIN "A25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SPI_DQ[1] }]    ;                   # IO_L1N_T0_D01_DIN_14          Sch = SPI_DQ1
set_property -dict { PACKAGE_PIN "B22"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SPI_DQ[2] }]    ;                   # IO_L2P_T0_D02_14              Sch = SPI_DQ2
set_property -dict { PACKAGE_PIN "A22"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SPI_DQ[3] }]    ;                   # IO_L2N_T0_D03_14              Sch = SPI_DQ3
set_property -dict { PACKAGE_PIN "C8"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SPI_SCK }]      ;                   # CCLK_0                        Sch = SPI_SCK


####################################################################################################################
#                                              MicroSD                                                             #
####################################################################################################################
set_property -dict { PACKAGE_PIN "J23"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SD_D2  }]   ;                       # IO_L24N_T3_A00_D16_14         Sch = SD_D2
set_property -dict { PACKAGE_PIN "H23"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SD_CS  }]   ;                       # IO_L20P_T3_A08_D24_14         Sch = SD_CS
set_property -dict { PACKAGE_PIN "H24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SD_SDI }]   ;                       # IO_L20N_T3_A07_D23_14         Sch = SD_SDI
set_property -dict { PACKAGE_PIN "G22"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SD_SCK }]   ;                       # IO_L13P_T2_MRCC_14            Sch = SD_SCK
set_property -dict { PACKAGE_PIN "F25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SD_SDO }]   ;                       # IO_L17P_T2_A14_D30_14         Sch = SD_SDO
set_property -dict { PACKAGE_PIN "E25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SD_RSV }]   ;                       # IO_L15P_T2_DQS_RDWR_B_14      Sch = SD_RSV


####################################################################################################################
#                                              RGB LED                                                             #
####################################################################################################################
set_property -dict { PACKAGE_PIN "J26"   IOSTANDARD LVCMOS33        SLEW FAST} [get_ports { LED[0] }]   ;                       # IO_L18P_T2_A12_D28_14         Sch = LED0
set_property -dict { PACKAGE_PIN "H26"   IOSTANDARD LVCMOS33        SLEW FAST} [get_ports { LED[1] }]   ;                       # IO_L18N_T2_A11_D27_14         Sch = LED1
set_property -dict { PACKAGE_PIN "G26"   IOSTANDARD LVCMOS33        SLEW FAST} [get_ports { LED[2] }]   ;                       # IO_L16N_T2_A15_D31_14         Sch = LED2


####################################################################################################################
#                                              FMC GTP                                                             #
####################################################################################################################
set_property -dict { PACKAGE_PIN "G3"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP0_M2C_N }]    ;                   # MGTXRXN0_116                  Sch = DP0_M2C_N
set_property -dict { PACKAGE_PIN "G4"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP0_M2C_P }]    ;                   # MGTXRXP0_116                  Sch = DP0_M2C_P
set_property -dict { PACKAGE_PIN "E3"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP1_M2C_N }]    ;                   # MGTXRXN1_116                  Sch = DP1_M2C_N
set_property -dict { PACKAGE_PIN "E4"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP1_M2C_P }]    ;                   # MGTXRXP1_116                  Sch = DP1_M2C_P
set_property -dict { PACKAGE_PIN "C3"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP2_M2C_N }]    ;                   # MGTXRXN2_116                  Sch = DP2_M2C_N
set_property -dict { PACKAGE_PIN "C4"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP2_M2C_P }]    ;                   # MGTXRXP2_116                  Sch = DP2_M2C_P
set_property -dict { PACKAGE_PIN "B5"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP3_M2C_N }]    ;                   # MGTXRXN3_116                  Sch = DP3_M2C_N
set_property -dict { PACKAGE_PIN "B6"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP3_M2C_P }]    ;                   # MGTXRXP3_116                  Sch = DP3_M2C_P


set_property -dict { PACKAGE_PIN "D5"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { GBTCLK1_M2C_N}]   ;                 # MGTREFCLK0N_116               Sch = GBTCLK1_M2C_N
set_property -dict { PACKAGE_PIN "D6"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { GBTCLK1_M2C_P}]   ;                 # MGTREFCLK0P_116               Sch = GBTCLK1_M2C_P
set_property -dict { PACKAGE_PIN "H5"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MGTREFCLK1_N }]   ;                 # MGTREFCLK0N_115               Sch = MGTREFCLK1_N
set_property -dict { PACKAGE_PIN "H6"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MGTREFCLK1_P }]   ;                 # MGTREFCLK0P_115               Sch = MGTREFCLK1_P
set_property -dict { PACKAGE_PIN "F6"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { GBTCLK0_M2C_P}]   ;                 # MGTREFCLK1P_116               Sch = GBTCLK0_M2C_P
set_property -dict { PACKAGE_PIN "F5"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { GBTCLK0_M2C_N}]   ;                 # MGTREFCLK1N_116               Sch = GBTCLK0_M2C_N


set_property -dict { PACKAGE_PIN "F2"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP0_C2M_P }]    ;                   # MGTTXTXP0_116                 Sch = DP0_C2M_P
set_property -dict { PACKAGE_PIN "F1"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP0_C2M_N }]    ;                   # MGTTXTXN0_116                 Sch = DP0_C2M_N
set_property -dict { PACKAGE_PIN "D2"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP1_C2M_P }]    ;                   # MGTTXTXP1_116                 Sch = DP1_C2M_P
set_property -dict { PACKAGE_PIN "D1"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP1_C2M_N }]    ;                   # MGTTXTXN1_116                 Sch = DP1_C2M_N
set_property -dict { PACKAGE_PIN "B2"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP2_C2M_P }]    ;                   # MGTTXTXP2_116                 Sch = DP2_C2M_P
set_property -dict { PACKAGE_PIN "B1"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP2_C2M_N }]    ;                   # MGTTXTXN2_116                 Sch = DP2_C2M_N
set_property -dict { PACKAGE_PIN "A4"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP3_C2M_P }]    ;                   # MGTTXTXP3_116                 Sch = DP3_C2M_P
set_property -dict { PACKAGE_PIN "A3"     IOSTANDARD LVCMOS15       SLEW FAST} [get_ports { DP3_C2M_N }]    ;                   # MGTTXTXN3_116                 Sch = DP3_C2M_N


####################################################################################################################
#                                              PCIe Pins                                                           #
####################################################################################################################

set_property -dict { PACKAGE_PIN "K6" } [get_ports { PCIe_REFCLK_P }]   ;                                                       # MGTREFCLK1P_115               Sch = REFCLK_P
set_property -dict { PACKAGE_PIN "K5" } [get_ports { PCIe_REFCLK_N }]   ;                                                       # MGTREFCLK1N_115               Sch = REFCLK_N

set_property -dict { PACKAGE_PIN "R4" } [get_ports { PCIe_RX_N[3] }]    ;                                                       # MGTTXRXP0_115                 Sch = PCIe_RX3_N
set_property -dict { PACKAGE_PIN "R3" } [get_ports { PCIe_RX_P[3] }]    ;                                                       # MGTTXRXN0_115                 Sch = PCIe_RX3_P
set_property -dict { PACKAGE_PIN "N4" } [get_ports { PCIe_RX_N[2] }]    ;                                                       # MGTTXRXP1_115                 Sch = PCIe_RX2_N
set_property -dict { PACKAGE_PIN "N3" } [get_ports { PCIe_RX_P[2] }]    ;                                                       # MGTTXRXN1_115                 Sch = PCIe_RX2_P
set_property -dict { PACKAGE_PIN "L4" } [get_ports { PCIe_RX_N[1] }]    ;                                                       # MGTTXRXP2_115                 Sch = PCIe_RX1_N
set_property -dict { PACKAGE_PIN "L3" } [get_ports { PCIe_RX_P[1] }]    ;                                                       # MGTTXRXN2_115                 Sch = PCIe_RX1_P
set_property -dict { PACKAGE_PIN "J4" } [get_ports { PCIe_RX_N[0] }]    ;                                                       # MGTTXRXP3_115                 Sch = PCIe_RX0_N
set_property -dict { PACKAGE_PIN "J3" } [get_ports { PCIe_RX_P[0] }]    ;                                                       # MGTTXRXN3_115                 Sch = PCIe_RX0_P

set_property -dict { PACKAGE_PIN "P2" } [get_ports { PCIe_TX_N[3] }]    ;                                                       # MGTTXTXP0_115                 Sch = PCIe_TX3_N
set_property -dict { PACKAGE_PIN "P1" } [get_ports { PCIe_TX_P[3] }]    ;                                                       # MGTTXTXN0_115                 Sch = PCIe_TX3_P
set_property -dict { PACKAGE_PIN "M2" } [get_ports { PCIe_TX_N[2] }]    ;                                                       # MGTTXTXP1_115                 Sch = PCIe_TX2_N
set_property -dict { PACKAGE_PIN "M1" } [get_ports { PCIe_TX_P[2] }]    ;                                                       # MGTTXTXN1_115                 Sch = PCIe_TX2_P
set_property -dict { PACKAGE_PIN "K2" } [get_ports { PCIe_TX_N[1] }]    ;                                                       # MGTTXTXP2_115                 Sch = PCIe_TX1_N
set_property -dict { PACKAGE_PIN "K1" } [get_ports { PCIe_TX_P[1] }]    ;                                                       # MGTTXTXN2_115                 Sch = PCIe_TX1_P
set_property -dict { PACKAGE_PIN "H2" } [get_ports { PCIe_TX_N[0] }]    ;                                                       # MGTTXTXP3_115                 Sch = PCIe_TX0_N
set_property -dict { PACKAGE_PIN "H1" } [get_ports { PCIe_TX_P[0] }]    ;                                                       # MGTTXTXN3_115                 Sch = PCIe_TX0_P

set_property -dict { PACKAGE_PIN "E21" } [get_ports { PCIe_PERST# }];                                                           # IO_L9P_T1_DQS_14              Sch = PCIe_PERST#


####################################################################################################################
#                                              FMC GPIO                                                            #
####################################################################################################################
set_property -dict { PACKAGE_PIN "AA23"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MLVDS_DI_0          }]   ;          # IO_L11P_T1_SRCC_12            Sch = LA00_CC_P         FMC = G6
set_property -dict { PACKAGE_PIN "AB24"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MLDVS_RO_0          }]   ;          # IO_L11N_T1_SRCC_12            Sch = LA00_CC_N         FMC = G7
set_property -dict { PACKAGE_PIN "Y23"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA01_P              }]   ;          # IO_L12P_T1_MRCC_12            Sch = LA01_CC_P         FMC = D8
set_property -dict { PACKAGE_PIN "AA24"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA01_N              }]   ;          # IO_L12N_T1_MRCC_12            Sch = LA01_CC_N         FMC = D9
set_property -dict { PACKAGE_PIN "AD26"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA02_P              }]   ;          # IO_L21P_T3_DQS_12             Sch = LA02_P            FMC = H7
set_property -dict { PACKAGE_PIN "AE26"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA02_N              }]   ;          # IO_L21N_T3_DQS_12             Sch = LA02_N            FMC = H8
set_property -dict { PACKAGE_PIN "AA25"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA03_P              }]   ;          # IO_L7P_T1_12                  Sch = LA03_P            FMC = G9
set_property -dict { PACKAGE_PIN "AB25"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT6            }]   ;          # IO_L7N_T1_12                  Sch = LA03_N            FMC = G10
set_property -dict { PACKAGE_PIN "AD25"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT11           }]   ;          # IO_L23P_T3_12                 Sch = LA04_P            FMC = H10
set_property -dict { PACKAGE_PIN "AE25"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA04_N              }]   ;          # IO_L23N_T3_12                 Sch = LA04_N            FMC = H11
set_property -dict { PACKAGE_PIN "W25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT9            }]   ;          # IO_L5P_T0_12                  Sch = LA05_P            FMC = D11
set_property -dict { PACKAGE_PIN "W26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA05_N              }]   ;          # IO_L5N_T0_12                  Sch = LA05_N            FMC = D12
set_property -dict { PACKAGE_PIN "Y25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DEBUG0              }]   ;          # IO_L10P_T1_12                 Sch = LA06_P            FMC = C10
set_property -dict { PACKAGE_PIN "Y26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DEBUG1              }]   ;          # IO_L10N_T1_12                 Sch = LA06_N            FMC = C11
set_property -dict { PACKAGE_PIN "V23"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { ADC_CONVST          }]   ;          # IO_L3P_T0_DQS_12              Sch = LA07_P            FMC = H13
set_property -dict { PACKAGE_PIN "V24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DAC_MOSI            }]   ;          # IO_L3N_T0_DQS_12              Sch = LA07_N            FMC = H14
set_property -dict { PACKAGE_PIN "U26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA08_P              }]   ;          # IO_L4P_T0_12                  Sch = LA08_P            FMC = G12
set_property -dict { PACKAGE_PIN "V26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA08_N              }]   ;          # IO_L4N_T0_12                  Sch = LA08_N            FMC = G13
set_property -dict { PACKAGE_PIN "W20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DEBUG2              }]   ;          # IO_L15P_T2_DQS_12             Sch = LA09_P            FMC = D14
set_property -dict { PACKAGE_PIN "Y21"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DEBUG4              }]   ;          # IO_L15N_T2_DQS_12             Sch = LA09_N            FMC = D15
set_property -dict { PACKAGE_PIN "V21"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DEBUG3              }]   ;          # IO_L6P_T0_12                  Sch = LA10_P            FMC = C14
set_property -dict { PACKAGE_PIN "W21"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DEBUG5              }]   ;          # IO_L6N_T0_VREF_12             Sch = LA10_N            FMC = C15
set_property -dict { PACKAGE_PIN "L19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA11_P              }]   ;          # IO_L21P_T3_DQS_15             Sch = LA11_P            FMC = H16
set_property -dict { PACKAGE_PIN "L20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA11_N              }]   ;          # IO_L21N_T3_DQS_A18_15         Sch = LA11_N            FMC = H17
set_property -dict { PACKAGE_PIN "M17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA12_P              }]   ;          # IO_L23P_T3_FOE_B_15           Sch = LA12_P            FMC = G15
set_property -dict { PACKAGE_PIN "L18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA12_N              }]   ;          # IO_L23N_T3_FWE_B_15           Sch = LA12_N            FMC = G16
set_property -dict { PACKAGE_PIN "K20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA13_P              }]   ;          # IO_L19P_T3_A22_15             Sch = LA13_P            FMC = D17
set_property -dict { PACKAGE_PIN "J20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA13_N              }]   ;          # IO_L19N_T3_A21_VREF_15        Sch = LA13_N            FMC = D18
set_property -dict { PACKAGE_PIN "J18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DEBUG6              }]   ;          # IO_L20P_T3_A20_15             Sch = LA14_P            FMC = C18
set_property -dict { PACKAGE_PIN "J19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DEBUG7              }]   ;          # IO_L20N_T3_A19_15             Sch = LA14_N            FMC = C19
set_property -dict { PACKAGE_PIN "U17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT0            }]   ;          # IO_L23P_T3_13                 Sch = LA15_P            FMC = H19
set_property -dict { PACKAGE_PIN "T17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA15_N              }]   ;          # IO_L23N_T3_13                 Sch = LA15_N            FMC = H20
set_property -dict { PACKAGE_PIN "T18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT2            }]   ;          # IO_L19P_T3_13                 Sch = LA16_P            FMC = G18
set_property -dict { PACKAGE_PIN "T19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT1            }]   ;          # IO_L19N_T3_VREF_13            Sch = LA16_N            FMC = G19
set_property -dict { PACKAGE_PIN "E18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DO0                 }]   ;          # IO_L13P_T2_MRCC_15            Sch = LA17_CC_P         FMC = D20
set_property -dict { PACKAGE_PIN "D18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DI0                 }]   ;          # IO_L13N_T2_MRCC_15            Sch = LA17_CC_N         FMC = D21
set_property -dict { PACKAGE_PIN "F17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_PCLK_B          }]   ;          # IO_L12P_T1_MRCC_AD5P_15       Sch = LA18_CC_P         FMC = C22
set_property -dict { PACKAGE_PIN "E17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA18_N              }]   ;          # IO_L12N_T1_MRCC_AD5N_15       Sch = LA18_CC_N         FMC = C23
set_property -dict { PACKAGE_PIN "H16"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_SDA_B           }]   ;          # IO_L7P_T1_AD10P_15            Sch = LA19_P            FMC = H22
set_property -dict { PACKAGE_PIN "G16"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_HSYNC_B         }]   ;          # IO_L7N_T1_AD10N_15            Sch = LA19_N            FMC = H23
set_property -dict { PACKAGE_PIN "K16"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_LEDG_B          }]   ;          # IO_L22P_T3_A17_15             Sch = LA20_P            FMC = G21
set_property -dict { PACKAGE_PIN "K17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_SCL_B           }]   ;          # IO_L22N_T3_A16_15             Sch = LA20_N            FMC = G22
set_property -dict { PACKAGE_PIN "D19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA21_P              }]   ;          # IO_L15P_T2_DQS_15             Sch = LA21_P            FMC = H25
set_property -dict { PACKAGE_PIN "D20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D6_B            }]   ;          # IO_L15N_T2_DQS_ADV_B_15       Sch = LA21_N            FMC = H26
set_property -dict { PACKAGE_PIN "C19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D10_B           }]   ;          # IO_L4P_T0_AD9P_15             Sch = LA22_P            FMC = G24
set_property -dict { PACKAGE_PIN "B19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA22_N              }]   ;          # IO_L4N_T0_AD9N_15             Sch = LA22_N            FMC = G25
set_property -dict { PACKAGE_PIN "C17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D11_B           }]   ;          # IO_L5P_T0_AD2P_15             Sch = LA23_P            FMC = D23
set_property -dict { PACKAGE_PIN "C18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LDES_D9_B           }]   ;          # IO_L5N_T0_AD2N_15             Sch = LA23_N            FMC = D24
set_property -dict { PACKAGE_PIN "D15"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D2_B            }]   ;          # IO_L6P_T0_15                  Sch = LA24_P            FMC = H28
set_property -dict { PACKAGE_PIN "D16"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D0_B            }]   ;          # IO_L6N_T0_VREF_15             Sch = LA24_N            FMC = H29
set_property -dict { PACKAGE_PIN "F19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D3_B            }]   ;          # IO_L17P_T2_A26_15             Sch = LA25_P            FMC = G27
set_property -dict { PACKAGE_PIN "J15"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D5_B            }]   ;          # IO_L9P_T1_DQS_AD11P_15        Sch = LA26_P            FMC = D26
set_property -dict { PACKAGE_PIN "J16"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D4_B            }]   ;          # IO_L9N_T1_DQS_AD11N_15        Sch = LA26_N            FMC = D27
set_property -dict { PACKAGE_PIN "G15"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA27_P              }]   ;          # IO_L8P_T1_AD3P_15             Sch = LA27_P            FMC = C26
set_property -dict { PACKAGE_PIN "F15"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA27_N              }]   ;          # IO_L8N_T1_AD3N_15             Sch = LA27_N            FMC = C27
set_property -dict { PACKAGE_PIN "G17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA28_P              }]   ;          # IO_L11P_T1_SRCC_AD12P_15      Sch = LA28_P            FMC = H31
set_property -dict { PACKAGE_PIN "F18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_GPIO0_B         }]   ;          # IO_L11N_T1_SRCC_AD12N_15      Sch = LA28_N            FMC = H32
set_property -dict { PACKAGE_PIN "E15"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_GPIO3_B         }]   ;          # IO_L10P_T1_AD4P_15            Sch = LA29_P            FMC = G30
set_property -dict { PACKAGE_PIN "E16"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_GPIO1_B         }]   ;          # IO_L10N_T1_AD4N_15            Sch = LA29_N            FMC = G31
set_property -dict { PACKAGE_PIN "H17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA30_P              }]   ;          # IO_L14P_T2_SRCC_15            Sch = LA30_P            FMC = H34
set_property -dict { PACKAGE_PIN "H18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA30_N              }]   ;          # IO_L14N_T2_SRCC_15            Sch = LA30_N            FMC = H35
set_property -dict { PACKAGE_PIN "G19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_PDN_B           }]   ;          # IO_L16P_T2_A28_15             Sch = LA31_P            FMC = G33
set_property -dict { PACKAGE_PIN "F20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA31_N              }]   ;          # IO_L16N_T2_A27_15             Sch = LA31_N            FMC = G34
set_property -dict { PACKAGE_PIN "H19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LINK_A_PWR_EN       }]   ;          # IO_L18P_T2_A24_15             Sch = LA32_P            FMC = H37
set_property -dict { PACKAGE_PIN "G20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LINK_PWR_ADJ_SCLK   }]   ;          # IO_L18N_T2_A23_15             Sch = LA32_N            FMC = H38
set_property -dict { PACKAGE_PIN "L17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA33_P              }]   ;          # IO_L24P_T3_RS1_15             Sch = LA33_P            FMC = G36
set_property -dict { PACKAGE_PIN "K18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LA33_N              }]   ;          # IO_L24N_T3_RS0_15             Sch = LA33_N            FMC = G37
set_property -dict { PACKAGE_PIN "P23"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MLVDS_DI_1          }]   ;          # IO_L11P_T1_SRCC_13            Sch = HA00_CC_P         FMC = F4
set_property -dict { PACKAGE_PIN "N23"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MLDVS_RO_1          }]   ;          # IO_L11N_T1_SRCC_13            Sch = HA00_CC_N         FMC = F5
set_property -dict { PACKAGE_PIN "N21"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MLVDS_DI_2          }]   ;          # IO_L12P_T1_MRCC_13            Sch = HA01_CC_P         FMC = E2
set_property -dict { PACKAGE_PIN "N22"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MLDVS_RO_2          }]   ;          # IO_L12N_T1_MRCC_13            Sch = HA01_CC_N         FMC = E3
set_property -dict { PACKAGE_PIN "AB22"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { CAN0-TX             }]   ;          # IO_L17P_T2_12                 Sch = HA02_P            FMC = K7
set_property -dict { PACKAGE_PIN "AC22"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { CAN0-RX             }]   ;          # IO_L17N_T2_12                 Sch = HA02_N            FMC = K8
set_property -dict { PACKAGE_PIN "AD23"   IOSTANDARD TMDS_33        SLEW FAST} [get_ports { TP4_P               }]   ;          # IO_L16P_T2_12                 Sch = HA03_P            FMC = J6
set_property -dict { PACKAGE_PIN "AD24"   IOSTANDARD TMDS_33        SLEW FAST} [get_ports { TP4_N               }]   ;          # IO_L16N_T2_12                 Sch = HA03_N            FMC = J7
set_property -dict { PACKAGE_PIN "N19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA04_P              }]   ;          # IO_L7P_T1_13                  Sch = HA04_P            FMC = F7
set_property -dict { PACKAGE_PIN "M20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA04_N              }]   ;          # IO_L7N_T1_13                  Sch = HA04_N            FMC = F8
set_property -dict { PACKAGE_PIN "R18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA05_P              }]   ;          # IO_L24P_T3_13                 Sch = HA05_P            FMC = E6
set_property -dict { PACKAGE_PIN "P18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA05_N              }]   ;          # IO_L24N_T3_13                 Sch = HA05_N            FMC = E7
set_property -dict { PACKAGE_PIN "P16"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { ADC_MOSI            }]   ;          # IO_L20P_T3_13                 Sch = HA06_P            FMC = K10
set_property -dict { PACKAGE_PIN "N17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { ADC_MISO_A          }]   ;          # IO_L20N_T3_13                 Sch = HA06_N            FMC = K11
set_property -dict { PACKAGE_PIN "R16"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT5            }]   ;          # IO_L21P_T3_DQS_13             Sch = HA07_P            FMC = J9
set_property -dict { PACKAGE_PIN "R17"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { ADC_MISO_B          }]   ;          # IO_L21N_T3_DQS_13             Sch = HA07_N            FMC = J10
set_property -dict { PACKAGE_PIN "U19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { nADC_RESET          }]   ;          # IO_L18P_T2_13                 Sch = HA08_P            FMC = F10
set_property -dict { PACKAGE_PIN "U20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT8            }]   ;          # IO_L18N_T2_13                 Sch = HA08_N            FMC = F11
set_property -dict { PACKAGE_PIN "N18"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA09_P              }]   ;          # IO_L22P_T3_13                 Sch = HA09_P            FMC = E9
set_property -dict { PACKAGE_PIN "M19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA09_N              }]   ;          # IO_L22N_T3_13                 Sch = HA09_N            FMC = E10
set_property -dict { PACKAGE_PIN "T20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { nADC_CS             }]   ;          # IO_L16P_T2_13                 Sch = HA10_P            FMC = K13
set_property -dict { PACKAGE_PIN "R20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA10_N              }]   ;          # IO_L16N_T2_13                 Sch = HA10_N            FMC = K14
set_property -dict { PACKAGE_PIN "P19"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { ADC_BUSY            }]   ;          # IO_L9P_T1_DQS_13              Sch = HA11_P            FMC = J12
set_property -dict { PACKAGE_PIN "P20"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { ADC_SCLK            }]   ;          # IO_L9N_T1_DQS_13              Sch = HA11_N            FMC = J13
set_property -dict { PACKAGE_PIN "T24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA12_P              }]   ;          # IO_L15P_T2_DQS_13             Sch = HA12_P            FMC = F13
set_property -dict { PACKAGE_PIN "T25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { nDAC_RESET          }]   ;          # IO_L15N_T2_DQS_13             Sch = HA12_N            FMC = F14
set_property -dict { PACKAGE_PIN "U24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT10           }]   ;          # IO_L2P_T0_12                  Sch = HA13_P            FMC = E12
set_property -dict { PACKAGE_PIN "U25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA13_N              }]   ;          # IO_L2N_T0_12                  Sch = HA13_N            FMC = E13
set_property -dict { PACKAGE_PIN "R26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DAC_SCLK            }]   ;          # IO_L2P_T0_13                  Sch = HA14_P            FMC = J15
set_property -dict { PACKAGE_PIN "P26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { nDAC_CS             }]   ;          # IO_L2N_T0_13                  Sch = HA14_N            FMC = J16
set_property -dict { PACKAGE_PIN "P24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA15_P              }]   ;          # IO_L4P_T0_13                  Sch = HA15_P            FMC = F16
set_property -dict { PACKAGE_PIN "N24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT7            }]   ;          # IO_L4N_T0_13                  Sch = HA15_N            FMC = F17
set_property -dict { PACKAGE_PIN "R25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA16_P              }]   ;          # IO_L6P_T0_13                  Sch = HA16_P            FMC = E15
set_property -dict { PACKAGE_PIN "P25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA16_N              }]   ;          # IO_L6N_T0_VREF_13             Sch = HA16_N            FMC = E16
set_property -dict { PACKAGE_PIN "M21"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DAC_MISO            }]   ;          # IO_L10P_T1_13                 Sch = HA17_CC_P         FMC = K16
set_property -dict { PACKAGE_PIN "M22"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DI1_TERM            }]   ;          # IO_L10N_T1_13                 Sch = HA17_CC_N         FMC = K17
set_property -dict { PACKAGE_PIN "N26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT3            }]   ;          # IO_L5P_T0_13                  Sch = HA18_P            FMC = J18
set_property -dict { PACKAGE_PIN "M26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { !AIO_EN             }]   ;          # IO_L5N_T0_13                  Sch = HA18_N            FMC = J19
set_property -dict { PACKAGE_PIN "K25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA19_P              }]   ;          # IO_L1P_T0_13                  Sch = HA19_P            FMC = F19
set_property -dict { PACKAGE_PIN "K26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HA19_N              }]   ;          # IO_L1N_T0_13                  Sch = HA19_N            FMC = F20
set_property -dict { PACKAGE_PIN "M25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { AIN/OUT4            }]   ;          # IO_L3P_T0_DQS_13              Sch = HA20_P            FMC = E18
set_property -dict { PACKAGE_PIN "L25"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DI1                 }]   ;          # IO_L3N_T0_DQS_13              Sch = HA20_N            FMC = E19
set_property -dict { PACKAGE_PIN "M24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DI0_TERM            }]   ;          # IO_L8P_T1_13                  Sch = HA21_P            FMC = K19
set_property -dict { PACKAGE_PIN "L24"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { VBIAS_SDA           }]   ;          # IO_L8N_T1_13                  Sch = HA21_N            FMC = K20
set_property -dict { PACKAGE_PIN "T22"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { VBIAS_SCL           }]   ;          # IO_L17P_T2_13                 Sch = HA22_P            FMC = J21
set_property -dict { PACKAGE_PIN "T23"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_LOCK_B          }]   ;          # IO_L17N_T2_13                 Sch = HA22_N            FMC = J22
set_property -dict { PACKAGE_PIN "U22"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_PASS_B          }]   ;          # IO_L1P_T0_12                  Sch = HA23_P            FMC = K22
set_property -dict { PACKAGE_PIN "V22"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_VSYNC_B         }]   ;          # IO_L1N_T0_12                  Sch = HA23_N            FMC = K23
set_property -dict { PACKAGE_PIN "E10"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB00_P              }]   ;          # IO_L12P_T1_MRCC_16            Sch = HB00_CC_P         FMC = K25
set_property -dict { PACKAGE_PIN "D10"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB00_N              }]   ;          # IO_L12N_T1_MRCC_16            Sch = HB00_CC_N         FMC = K26
set_property -dict { PACKAGE_PIN "F14"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB01_P              }]   ;          # IO_L15P_T2_DQS_16             Sch = HB01_P            FMC = J24
set_property -dict { PACKAGE_PIN "F13"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB01_N              }]   ;          # IO_L15N_T2_DQS_16             Sch = HB01_N            FMC = J25
set_property -dict { PACKAGE_PIN "H14"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB02_P              }]   ;          # IO_L5P_T0_16                  Sch = HB02_P            FMC = F22
set_property -dict { PACKAGE_PIN "G14"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB02_N              }]   ;          # IO_L5N_T0_16                  Sch = HB02_N            FMC = F23
set_property -dict { PACKAGE_PIN "J13"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB03_P              }]   ;          # IO_L3P_T0_DQS_16              Sch = HB03_P            FMC = E21
set_property -dict { PACKAGE_PIN "H13"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB03_N              }]   ;          # IO_L3N_T0_DQS_16              Sch = HB03_N            FMC = E22
set_property -dict { PACKAGE_PIN "B14"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB04_P              }]   ;          # IO_L21P_T3_DQS_16             Sch = HB04_P            FMC = F25
set_property -dict { PACKAGE_PIN "A14"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB04_N              }]   ;          # IO_L21N_T3_DQS_16             Sch = HB04_N            FMC = F26
set_property -dict { PACKAGE_PIN "B15"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB05_P              }]   ;          # IO_L23P_T3_16                 Sch = HB05_P            FMC = E24
set_property -dict { PACKAGE_PIN "A15"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB05_N              }]   ;          # IO_L23N_T3_16                 Sch = HB05_N            FMC = E25
set_property -dict { PACKAGE_PIN "C12"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB06_P              }]   ;          # IO_L13P_T2_MRCC_16            Sch = HB06_P            FMC = K28
set_property -dict { PACKAGE_PIN "C11"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_PCLK_A          }]   ;          # IO_L13N_T2_MRCC_16            Sch = HB06_N            FMC = K29
set_property -dict { PACKAGE_PIN "G10"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB07_P              }]   ;          # IO_L2P_T0_16                  Sch = HB07_P            FMC = J27
set_property -dict { PACKAGE_PIN "G9"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_PASS_A          }]   ;          # IO_L2N_T0_16                  Sch = HB07_N            FMC = J28
set_property -dict { PACKAGE_PIN "E13"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_LEDG_A          }]   ;          # IO_L18P_T2_16                 Sch = HB08_P            FMC = F28
set_property -dict { PACKAGE_PIN "E12"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_SCL_A           }]   ;          # IO_L18N_T2_16                 Sch = HB08_N            FMC = F29
set_property -dict { PACKAGE_PIN "D14"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_LOCK_A          }]   ;          # IO_L17P_T2_16                 Sch = HB09_P            FMC = E27
set_property -dict { PACKAGE_PIN "D13"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_SDA_A           }]   ;          # IO_L17N_T2_16                 Sch = HB09_N            FMC = E28
set_property -dict { PACKAGE_PIN "C9"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D11_A           }]   ;          # IO_L10P_T1_16                 Sch = HB10_P            FMC = K31
set_property -dict { PACKAGE_PIN "B9"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D8_A            }]   ;          # IO_L10N_T1_16                 Sch = HB10_N            FMC = K32
set_property -dict { PACKAGE_PIN "A13"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_VSYNC_A         }]   ;          # IO_L24P_T3_16                 Sch = HB11_P            FMC = J30
set_property -dict { PACKAGE_PIN "A12"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D10_A           }]   ;          # IO_L24N_T3_16                 Sch = HB11_N            FMC = J31
set_property -dict { PACKAGE_PIN "B10"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D9_A            }]   ;          # IO_L22P_T3_16                 Sch = HB12_P            FMC = F31
set_property -dict { PACKAGE_PIN "A10"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D6_A            }]   ;          # IO_L22N_T3_16                 Sch = HB12_N            FMC = F32
set_property -dict { PACKAGE_PIN "B12"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_HSYNC_A         }]   ;          # IO_L20P_T3_16                 Sch = HB13_P            FMC = E30
set_property -dict { PACKAGE_PIN "B11"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D7_A            }]   ;          # IO_L20N_T3_16                 Sch = HB13_N            FMC = E31
set_property -dict { PACKAGE_PIN "F9"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D3_A            }]   ;          # IO_L7P_T1_16                  Sch = HB14_P            FMC = K34
set_property -dict { PACKAGE_PIN "F8"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB14_N              }]   ;          # IO_L7N_T1_16                  Sch = HB14_N            FMC = K35
set_property -dict { PACKAGE_PIN "E11"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D5_A            }]   ;          # IO_L14P_T2_SRCC_16            Sch = HB15_P            FMC = J33
set_property -dict { PACKAGE_PIN "D11"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D2_A            }]   ;          # IO_L14N_T2_SRCC_16            Sch = HB15_N            FMC = J34
set_property -dict { PACKAGE_PIN "D9"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D0_A            }]   ;          # IO_L8P_T1_16                  Sch = HB16_P            FMC = F34
set_property -dict { PACKAGE_PIN "D8"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_PDN_A           }]   ;          # IO_L8N_T1_16                  Sch = HB16_N            FMC = F35
set_property -dict { PACKAGE_PIN "G11"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_GPIO1_A         }]   ;          # IO_L11P_T1_SRCC_16            Sch = HB17_CC_P         FMC = K37
set_property -dict { PACKAGE_PIN "F10"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LINK_B_PWR_EN       }]   ;          # IO_L11N_T1_SRCC_16            Sch = HB17_CC_N         FMC = K38
set_property -dict { PACKAGE_PIN "G12"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_GPIO2_A         }]   ;          # IO_L16P_T2_16                 Sch = HB18_P            FMC = J36
set_property -dict { PACKAGE_PIN "F12"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_GPIO0_A         }]   ;          # IO_L16N_T2_16                 Sch = HB18_N            FMC = J37
set_property -dict { PACKAGE_PIN "A9"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_D4_A            }]   ;          # IO_L9P_T1_DQS_16              Sch = HB19_P            FMC = E33
set_property -dict { PACKAGE_PIN "A8"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { DES_GPIO3_A         }]   ;          # IO_L9N_T1_DQS_16              Sch = HB19_N            FMC = E34
set_property -dict { PACKAGE_PIN "J11"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { nLINK_PWR_ADJ_RESET }]   ;          # IO_L4P_T0_16                  Sch = HB20_P            FMC = F37
set_property -dict { PACKAGE_PIN "J10"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { LINK_PWR_ADJ_MISO   }]   ;          # IO_L4N_T0_16                  Sch = HB20_N            FMC = F38
set_property -dict { PACKAGE_PIN "H9"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { HB21_P              }]   ;          # IO_L1P_T0_16                  Sch = HB21_P            FMC = E36
set_property -dict { PACKAGE_PIN "H8"     IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { nLINK_PWR_ADJ_CS    }]   ;          # IO_L1N_T0_16                  Sch = HB21_N            FMC = E37

set_property -dict { PACKAGE_PIN "R21"    IOSTANDARD TMDS_33        SLEW FAST} [get_ports { TP2_P               }]   ;          # IO_L13P_T2_MRCC_13            Sch = CLK3_BIDIR_P      FMC = J2
set_property -dict { PACKAGE_PIN "P21"    IOSTANDARD TMDS_33        SLEW FAST} [get_ports { TP2_N               }]   ;          # IO_L13N_T2_MRCC_13            Sch = CLK3_BIDIR_N      FMC = J3
set_property -dict { PACKAGE_PIN "R22"    IOSTANDARD TMDS_33        SLEW FAST} [get_ports { TP1_P               }]   ;          # IO_114P_T2_SRCC_13            Sch = CLK2_BIDIR_P      FMC = K5
set_property -dict { PACKAGE_PIN "R23"    IOSTANDARD TMDS_33        SLEW FAST} [get_ports { TP1_N               }]   ;          # IO_L14N_T2_SRCC_13            Sch = CLK2_BIDIR_N      FMC = K4
set_property -dict { PACKAGE_PIN "AC23"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MLVDS_DI_3          }]   ;          # IO_L14P_T2_SRCC_12            Sch = CLK1_M2C_P        FMC = G2
set_property -dict { PACKAGE_PIN "AC24"   IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { MLDVS_RO_3          }]   ;          # IO_L14N_T2_SRCC_12            Sch = CLK1_M2C_N        FMC = G3
set_property -dict { PACKAGE_PIN "Y22"    IOSTANDARD TMDS_33        SLEW FAST} [get_ports { TP3_P               }]   ;          # IO_L13P_T2_MRCC_12            Sch = CLK0_M2C_P        FMC = H4
set_property -dict { PACKAGE_PIN "AA22"   IOSTANDARD TMDS_33        SLEW FAST} [get_ports { TP3_N               }]   ;          # IO_L13N_T2_MRCC_12            Sch = CLK0_M2C_N        FMC = H5
set_property -dict { PACKAGE_PIN "D26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { PG_C2M              }]   ;          # IO_L5P_T0_D06_14              Sch = PG_C2M            FMC = D1
set_property -dict { PACKAGE_PIN "E26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { PG_M2C              }]   ;          # IO_17N_T2_A13_D29_14          Sch = PG_M2C            FMC = F1
set_property -dict { PACKAGE_PIN "C21"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SCL                 }]   ;          # IO_L10P_T1_D14_14             Sch = FMC_SCL           FMC = C30
set_property -dict { PACKAGE_PIN "B21"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { SDA                 }]   ;          # IO_L19N_T3_VREF_16            Sch = VREF_B_M2C        FMC = C31
set_property -dict { PACKAGE_PIN "B26"    IOSTANDARD LVCMOS33       SLEW FAST} [get_ports { FMC_PRSNT           }]   ;          # IO_L3N_T0_DQS_EMCCLK_14       Sch = PRSNT_M2C_L       FMC = H2
