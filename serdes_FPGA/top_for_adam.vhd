----------------------------------------------------------------------------------
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;
library UNISIM;
use UNISIM.VComponents.all;

entity top_for_adam is
port(
	clk_in : in std_logic; --input clock 
	reset  : in std_logic; --reset
	dclk   : out std_logic; --a clock that tells you a data is ready
	dout   : out std_logic_vector(15 downto 0); --dout -> feed this directly to fifo 
	
	--LVDS outputs goes to channel A of the intan chip
	cs_p   : out std_logic; 
	cs_n   : out std_logic; 
	sclk_p : out std_logic; 
	sclk_n : out std_logic; 
	mosi_p : out std_logic; 
	mosi_n : out std_logic; 
		
	--LVDS inputs
	miso_p : in std_logic; 
	miso_n : in std_logic
);
end top_for_adam;

architecture Behavioral of top_for_adam is

signal clk2M, clk50M, clk10M, clk5M, clk1M, clk50K, clk500K, clk_spi, clk_pot_spi, clktest, clk2Hz, clk4Hz: std_logic; 
signal count_bit : unsigned(11 downto 0);
signal cs, sclk, mosi, miso, spi_start, data_lclk, mosi_dl : std_logic; 
signal miso_reg : std_logic_vector(15 downto 0); 
signal command, command_dl, pot_command, led_command: std_logic_vector(15 downto 0); 
signal pot_state, pot_config_enb: std_logic; 
signal cs_pot, sclk_pot, din_pot : std_logic; 
signal led_clk, led_data, led_latch :  std_logic;

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
         command_o : OUT  std_logic_vector(15 downto 0)
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
	miso_reg_o : out std_logic_vector(15 downto 0) 
);
end component;

begin

--internal signal mapped to pins
dclk <= data_lclk;
dout <= miso_reg;

--clock selection
clk_spi <= clk5M; --spi clock is 5MHz; for the intan chips

--LVDS mapping ==============================================================
--outputs
lvds_mosi_map : OBUFDS generic map(IOSTANDARD => "LVDS_33") port map(O  => mosi_p, OB => mosi_n, I  => mosi);
lvds_sclk_map : OBUFDS generic map(IOSTANDARD => "LVDS_33") port map(O  => sclk_p, OB => sclk_n, I  => sclk);
lvds_cs_map   : OBUFDS generic map(IOSTANDARD => "LVDS_33") port map(O  => cs_p, OB => cs_n, I  => cs);
--inputs
lvds_miso_map : IBUFGDS generic map (DIFF_TERM => FALSE, IBUF_LOW_PWR => TRUE, IOSTANDARD => "LVDS_33") port map (O => miso,  I => miso_p, IB => miso_n);

--clock dividers
-----------------------------------------------------------------------------
clk_div_50M: clk_div generic map(MAXD=>2) --from 100MHz to 50MHz
	port map(clk=>clk_in, reset=>reset,div=>2, div_clk=>clk50M); 

clk_div_10M: clk_div generic map(MAXD=>10) --from 50MHz to 10MHz 
	port map(clk=>clk50M, reset=>reset,div=>10, div_clk=>clk10M);

clk_div_5M: clk_div generic map(MAXD=>2) --from 10MHz to 5MHz
	port map(clk=>clk10M, reset=>reset,div=>2, div_clk=>clk5M); 
-----------------------------------------------------------------------------
--main statemachine
mainstatement: main_sm PORT MAP (
          clk_spi => clk_spi,
          reset => reset,
          miso_reg => miso_reg,
          data_lclkin => data_lclk,
          spi_start_o => spi_start,
          command_o => command
			 );


--SPI module------------------------------------------------------
SPI_intan: SPI_module  
	port map(	
				clk_spi => clk_spi, 
				reset => reset,
				spi_start => spi_start,
				command_in => command, --read from 40 to 44 registers
				--SPI inputs
				miso_i => miso,
				--SPI outputs  
				cs_o => cs,    
				sclk_o => sclk, --sclk is always 2x slower than clk_spi
				mosi_o => mosi,
				--data latch clock 
				data_lclk_o => data_lclk,
				miso_reg_o => miso_reg
				); 

end Behavioral;

