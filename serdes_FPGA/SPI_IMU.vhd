----------------------------------------------------------------------------------
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

-- current generate the address and command internally, for examination purpose only

entity SPI_IMU is
port (
		clk_imu_spi : in std_logic; 
		reset       : in std_logic; 
		cs_o   : out std_logic; 
		sclk_o : out std_logic;
		mosi_o : out std_logic; 
		miso_o : in std_logic
); 
end SPI_IMU;

architecture Behavioral of SPI_IMU is

begin

--signal mapping
cs_o <= cs; 
sclk_o <= sclk; 
mosi_o <= cmd_reg(15); --comd_reg is a shift register so bit 15 goes to MOSI output
data_lclk_o <= data_lclk; 
miso_reg_o <= miso_reg;

--SPI state machine 
SPI_proc: process(clk_spi, reset) 
begin 
	if (reset = '1') then 
		cs <= '0'; 
		sclk <= '0';
		data_lclk <= '0'; 
		miso_reg <= (others=>'0');
		spi_sm <= IDLE; 
		cmd_reg <= (others=>'0'); 
		sm_cnt <= (others=>'0');  --sm counter 
	elsif (falling_edge(clk_spi)) then --next state logic
		cs <= cs_next; 
		sclk <= sclk_next; 
		data_lclk <= data_lclk_next;
		miso_reg <= miso_reg_next; 		
		spi_sm <= spi_sm_next; 
		cmd_reg <= cmd_reg_next;
		sm_cnt <= sm_cnt_next;
	end if; 
end process; 

end Behavioral;

