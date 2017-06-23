----------------------------------------------------------------------------------
--Top module for Headstage SerDes FPGA
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;
library UNISIM;
use UNISIM.VComponents.all;

entity top is
port(
	clk_in : in std_logic;  --input clock form oscillator 
	reset  : in std_logic;  --master system reset 
	pclk   : out std_logic; --PCLK on the FPD chip
	dout   : out std_logic_vector(11 downto 0); --din on the serializer side of the FPD chip. 
	
	--LVDS outputs goes to the intan chips
	cs_p   : out std_logic; 
	cs_n   : out std_logic; 
	sclk_p : out std_logic; 
	sclk_n : out std_logic; 
	mosi_p : out std_logic; 
	mosi_n : out std_logic;
	
	--LVDS inputs for driving two 64 channel intan chips 
	miso_chip1_p : in std_logic;
	miso_chip1_n : in std_logic;
	miso_chip2_p : in std_logic;
	miso_chip2_n : in std_logic;
	
	--VS, HSYNC output - this is named as vsync and hsync just for consistency with the PCB layout 
	vsync_o : out std_logic; --signal one data block 
	hsync_o : out std_logic; --signals at only channel 0 for OE synchronization
	
	--POT SPI interface use to config digital POT for LED driver 
	cs_pot_o   : out std_logic; --LEDSPI2 --N5  
	sclk_pot_o : out std_logic; --LEDSPI0 --N4
	din_pot_o  :out std_logic;  --LEDSPI1 --P5
	
	--LED enable input signals
	LED_GPO_0 : in std_logic; 
	
	--LED SPI interface 
   led_clk_o : out std_logic; --LED2
   led_data_o : out std_logic; --LED0
   led_latch_o : out std_logic; --LED1
	
	--LED active output signals
	LED0_active : out std_logic; 
	LED1_active : out std_logic
);
end top;

architecture Behavioral of top is

signal clk84M, clk42M, clk21M, clk2M, clk50M, clk10M, clk5M, clk1M, clk50K, clk500K, clk_spi, clk_pot_spi, clktest, clk2Hz, clk4Hz: std_logic; 
signal count_bit : unsigned(11 downto 0);
signal cs, sclk, mosi, spi_start, data_lclk, mosi_dl : std_logic; 
signal miso_chip1, miso_chip2 : std_logic; 
signal miso_reg : std_logic_vector(15 downto 0); 
signal command, command_dl, pot_command, led_command: std_logic_vector(15 downto 0); 
signal pot_state, pot_config_enb: std_logic; 
signal cs_pot, sclk_pot, din_pot : std_logic; 
signal led_clk, led_data, led_latch :  std_logic;
signal data_rdy_pcie : std_logic; 
signal data_pcie_A, data_pcie_B, data_pcie_C, data_pcie_D : std_logic_vector(15 downto 0); 
signal hsync, vsync : std_logic;

--clock divider
component clk_div is
	generic (MAXD: natural:=5);
	port(
			 clk: in std_logic;
			 reset: in std_logic;
			 div: in integer range 0 to MAXD;
			 div_clk: out std_logic 
			 );
end component;

--main state machine 
component main_sm
    port(
         clk_spi : IN  std_logic;
         reset : IN  std_logic;
         miso_reg : IN  std_logic_vector(15 downto 0);
         data_lclkin : IN  std_logic;
         spi_start_o : OUT  std_logic;
         command_o : OUT  std_logic_vector(15 downto 0);
			hsync_o : out std_logic
			);
end component;

--SPI data merger unit 
component data_merge is
	 port(
		pclk  : in std_logic; 
		reset : in std_logic;
		data_rdy_pcie : in std_logic; --this is generated from the SPI interface. Here we must sample this line using 50MHz clock	
		vsync_o : out std_logic; 
		stream1 : in std_logic_vector(15 downto 0); 
		stream2 : in std_logic_vector(15 downto 0); 
		stream3 : in std_logic_vector(15 downto 0); 
		stream4 : in std_logic_vector(15 downto 0); 
		dout_o  :  out std_logic_vector(7 downto 0)
	 );
end component;

--"SPI" interface for the LED driver chip
component SPI_LEDdriver
    port(
         clk_spi : IN  std_logic;
         reset : IN  std_logic;
         write_start : IN  std_logic;
         command_in : IN  std_logic_vector(15 downto 0);
         led_clk_o : OUT  std_logic;
         led_data_o : OUT  std_logic;
         led_latch_o : OUT  std_logic
        );
end component;

--84MHz clock module
component pll
port
 (-- Clock in ports
  CLK_IN1           : in     std_logic;
  -- Clock out ports
  CLK_OUT1          : out    std_logic;
  -- Status and control signals
  RESET             : in     std_logic;
  LOCKED            : out    std_logic
 );
end component;

--SPI module 
component SPI_module is
port(
	clk_spi : in std_logic; --spi clock from toplevel 
	reset : in std_logic;   --reset 
	spi_start : in std_logic; --spi initiate 
	command_in : in std_logic_vector(15 downto 0); --parallel command input vector
	--SPI inputs
	miso_i : in std_logic; 
	--SPI outputs  
	cs_o : out std_logic;   
	sclk_o : out std_logic; --sclk is always 2x slower than clk_spi
	mosi_o : out std_logic;
	--data latch clock 
	data_lclk_o : out std_logic;
	data_rdy_pcie_o : out std_logic; 
	data_pcie_A_o : out std_logic_vector(15 downto 0); 
	data_pcie_B_o : out std_logic_vector(15 downto 0); 
	miso_reg_A_o : out std_logic_vector(15 downto 0);
	miso_reg_B_o : out std_logic_vector(15 downto 0) 
);
end component;

begin

--internal signal mapped to pins
pclk       <= clk42M;
cs_pot_o   <= cs_pot;
sclk_pot_o <= sclk_pot;
din_pot_o  <= din_pot;

--debug signals sent through last 4 LSB of dout
dout(1) <= '0'; 
dout(3) <= sclk;
dout(2) <= miso_chip1; 
dout(0) <= hsync; 

--h vsync
hsync_o <= hsync;
vsync_o <= vsync;

--led control 
led_clk_o   <= led_clk; 
led_data_o  <= led_data;
led_latch_o <= led_latch;

--clock selection
clk_spi <= clk42M; --clk42M;
clk_pot_spi <= clk500K; --for the digital pot

--LVDS mapping ==============================================================
--outputs
lvds_mosi_map : OBUFDS generic map(IOSTANDARD => "LVDS_33") port map(O  => mosi_p, OB => mosi_n, I  => mosi);
lvds_sclk_map : OBUFDS generic map(IOSTANDARD => "LVDS_33") port map(O  => sclk_p, OB => sclk_n, I  => sclk);
lvds_cs_map   : OBUFDS generic map(IOSTANDARD => "LVDS_33") port map(O  => cs_p, OB => cs_n, I  => cs);

--inputs
lvds_miso_chip1_map : IBUFGDS generic map (DIFF_TERM => FALSE, IBUF_LOW_PWR => TRUE, IOSTANDARD => "LVDS_33") port map (O => miso_chip1,  I => miso_chip1_p, IB => miso_chip1_n);
lvds_miso_chip2_map : IBUFGDS generic map (DIFF_TERM => FALSE, IBUF_LOW_PWR => TRUE, IOSTANDARD => "LVDS_33") port map (O => miso_chip2,  I => miso_chip2_p, IB => miso_chip2_n);

--clock dividers
-----------------------------------------------------------------------------
clk_div_84M: pll  --from 100MHz to 84MHz
	port map(CLK_IN1=>clk_in, reset=>reset,CLK_OUT1=>clk84M, LOCKED=>open);

clk_div_42M: clk_div generic map(MAXD=>2) --from 84MHz to 42MHz
	port map(clk=>clk84M, reset=>reset,div=>2, div_clk=>clk42M); 

clk_div_50M: clk_div generic map(MAXD=>2) --from 100MHz to 50MHz
	port map(clk=>clk84M, reset=>reset,div=>2, div_clk=>clk50M); 

clk_div_10M: clk_div generic map(MAXD=>5) --from 50MHz to 10MHz 
	port map(clk=>clk50M, reset=>reset,div=>5, div_clk=>clk10M);

clk_div_5M: clk_div generic map(MAXD=>2) --from 10MHz to 5MHz
	port map(clk=>clk10M, reset=>reset,div=>2, div_clk=>clk5M); 

clk_div_1M: clk_div generic map(MAXD=>5) --from 5MHz to 1MHz
	port map(clk=>clk5M, reset=>reset,div=>5, div_clk=>clk1M); --not a 50% duty cycle clock 

clk_div_500K: clk_div generic map(MAXD=>2) --from 1MHz to 500KHz
	port map(clk=>clk1M, reset=>reset,div=>2, div_clk=>clk500K); --not a 50% duty cycle clock 	

clk_div_debug_only: clk_div generic map(MAXD=>40) --from 5MHz to 1MHz
	port map(clk=>clk500K, reset=>reset,div=>40, div_clk=>clktest); --not a 50% duty cycle clock 	
-----------------------------------------------------------------------------

--map LED active to clk2Hz
LED0_active <= LED_GPO_0; 
LED1_active <= LED_GPO_0; 

mini_cnt_proc: process(spi_start, reset) 
begin
	if (reset = '1') then 
		count_bit <= (others=>'0'); 
	elsif (falling_edge(spi_start)) then
		count_bit <= count_bit + 1; 
	end if;
end process; 

--generate command 
--command <= "11" & std_logic_vector(to_unsigned(41,6)) & "00000000"; --read from 40 to 44 registers
--configuration sequence
--				  7654 3210 
--R0  0x80DE "1101 1110" 
--R1  0x8102 "0000 0010" -ADC buffer bias, 2 for >700 KS/s sampling rate. 
--R2  0x8204 "0000 0100" -MUX bias 4 for >700 KS/s sampling rate 
--R3  0x8302 "0000 0010" -digital out HiZ
--R4  0x845F "0101 1111" -MISO pull to highZ when CS is pulled high. twocomp. no absmode, DSP offset remove, k_freq = 0.000004857Hz
--R5  0x8500 "0000 0000" -disable impedance check 
--R6  0x8600 "0000 0000" -disable impedance check DAC
--R7  0x8700 "0000 0000" -disable impedance check amplifier
--R8  0x8811 "0001 0001" -RH1 DAC1: 17 upper cutoff 10KHz
--R9  0x8980 "1000 0000" -RH1 DAC2: 0  
--R10 0x8A10 "0001 0000" -RH2 DAC1: 16 
--R11 0x8B80 "1000 0000" -RH2 DAC2: 0
--R12 0x8C10 "0001 0000" -RL  DAC1
--R13 0x8DDC "1101 1100" -RL DAC2:28 DAC3:1 cutoff: 0.1HZ??????????????????????? confirm
--R14 0x8EFF "1111 1111" 
--R15 0x8FFF "1111 1111"
--R16 0x90FF "1111 1111"
--R17 0x91FF "1111 1111"

--main statemachine
--this state machine generates command and puts those commands into the SPI module to serialize to the headstage
mainstatement: main_sm PORT MAP (
          clk_spi => clk_spi,
          reset => reset,
          miso_reg => miso_reg,
          data_lclkin => data_lclk,
          spi_start_o => spi_start,
          command_o => command,
			 hsync_o => hsync
			 );
			 
--SPI data merger unit 
merge: data_merge port map (
		pclk  => clk42M, --this gets 50MHz, should be the same as pclk frequency 
		reset => reset, 
		data_rdy_pcie => data_rdy_pcie, --this is generated from the SPI interface. Here we must sample this line using 50MHz clock	
		vsync_o => vsync,  --link this directly to vsync_o output
		stream1 =>  data_pcie_A, 
		stream2 =>  data_pcie_B,
		stream3 =>  data_pcie_C,
		stream4 =>  data_pcie_D,
		dout_o  => dout(11 downto 4) ---debug
	 );

--SPI module------------------------------------------------------
SPI_intan_chip1: SPI_module  
	port map(	
				clk_spi => clk_spi, 
				reset => reset,
				spi_start => spi_start,
				command_in => command, --read from 40 to 44 registers
				--SPI inputs
				miso_i => miso_chip1,
				--SPI outputs  
				cs_o => cs,    
				sclk_o => sclk, --sclk is always 2x slower than clk_spi
				mosi_o => mosi,
				--data latch clock 
				data_lclk_o => data_lclk,
				data_rdy_pcie_o => data_rdy_pcie,			
				data_pcie_A_o => data_pcie_A,
				data_pcie_B_o => data_pcie_B, 
				miso_reg_A_o => miso_reg,
				miso_reg_B_o => open
				); 	
				
--SPI module------------------------------------------------------
SPI_intan_chip2: SPI_module  
	port map(	
				clk_spi => clk_spi, 
				reset => reset,
				spi_start => spi_start,
				command_in => command, --read from 40 to 44 registers
				--SPI inputs
				miso_i => miso_chip2,
				--SPI outputs  
				cs_o => open,    
				sclk_o => open, --sclk is always 2x slower than clk_spi
				mosi_o => open,
				--data latch clock 
				data_lclk_o => open,
				data_rdy_pcie_o => open,			
				data_pcie_A_o => data_pcie_C,
				data_pcie_B_o => data_pcie_D, 
				miso_reg_A_o => open,
				miso_reg_B_o => open
				); 	
				
--LED development-------------------------------------------------------------
--generate the one shot for configuration 
on_shot_pot: process(clk_pot_spi, reset) 
begin 
	if (reset = '1') then 
		pot_state <= '0'; 
		pot_config_enb <= '0'; 
	elsif (rising_edge(clk_pot_spi)) then 
		if pot_state = '0' then 
			pot_state <= '1';
			pot_config_enb <= '1'; 
		else 
			pot_state <= '1';
			pot_config_enb <= '0'; 
		end if; 
	end if;
end process; 				

--pot command: write wiper information to register A and B. 				
--[C1 C0]="00"
--[A1 A0]="11" 
pot_command <= "00" & "00" & "00" & "11" & "11011010"; --10K Ohm

--variable resistor (POT) SPI module--------------------------------------------------
SPI_imu: SPI_module  
	port map(	
				clk_spi => clk_pot_spi, --keep the frequency aroudn 1MHz or even slower
				reset => reset,
				spi_start => clktest, --generate a 1-shot configuration (get this from the main state_machine?)
				command_in => pot_command, --read from 40 to 44 registers
				--SPI inputs
				miso_i => '0', --ground the miso for the pot because there is no output
				--SPI outputs  
				cs_o => cs_pot,     
				sclk_o => sclk_pot, --sclk is always 2x slower than clk_spi
				mosi_o => din_pot,
				--data latch clock 
				data_lclk_o => open,
				data_rdy_pcie_o => open, 
				data_pcie_A_o => open,
				data_pcie_B_o => open, 
				miso_reg_A_o => open,
				miso_reg_B_o => open
				); 

--LED configuration command 
--led_command <= "1111111111111111"; --all on
led_command <= "0000000000000011"; 

-- Instantiate the Unit Under Test (UUT)
   leddirver: SPI_LEDdriver PORT MAP (
          clk_spi => clk_pot_spi,
          reset => reset,
          write_start => clktest,
          command_in => led_command,
          led_clk_o => led_clk,
          led_data_o => led_data,
          led_latch_o => led_latch
        );

end Behavioral;

