----------------------------------------------------------------------------------
--this is an parallel to serial converter
--takes command_in and serilize it for the LED driver
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity SPI_LEDdriver is
port (
	clk_spi     : in std_logic;
	reset       : in std_logic; 
	write_start : in std_logic;
   command_in  : in std_logic_vector(15 downto 0); 
	
	led_clk_o   : out std_logic; 
	led_data_o  : out std_logic; 
	led_latch_o     : out std_logic
); 
end SPI_LEDdriver;

architecture Behavioral of SPI_LEDdriver is

signal led_clk, led_latch : std_logic; 
signal led_clk_next, led_latch_next : std_logic; 
signal led_data, led_data_next : std_logic_vector(15 downto 0); 
type spi_states is (IDLE, OP_lo, OP_hi, LATCHRDY); --state machine definition
signal spi_sm, spi_sm_next : spi_states; 
signal sm_cnt, sm_cnt_next : unsigned(3 downto 0); 
signal cycle_cnt, cycle_cnt_next : unsigned(3 downto 0); 

begin

--ouput mapping 
led_clk_o <= led_clk; 
led_data_o <= led_data(15);
led_latch_o <= led_latch;

--SPI state machine 
SPI_proc: process(clk_spi, reset) 
begin 
	if (reset = '1') then 
		led_clk <= '0'; 
		led_data <= (others=>'0');
		led_latch <= '0'; 
		spi_sm <= IDLE; 
		sm_cnt <= (others=>'0'); 
		cycle_cnt <= (others=>'0'); 
	elsif (falling_edge(clk_spi)) then --next state logic
		led_clk <= led_clk_next; 
		led_data <= led_data_next;
		led_latch <= led_latch_next; 
		spi_sm <= spi_sm_next; 
		sm_cnt <= sm_cnt_next; 
		cycle_cnt <= cycle_cnt_next; 
	end if; 
end process; 

--next state logic for the state machines 
SPI_proc_next: process(spi_sm, sm_cnt, write_start, command_in, led_data, led_clk, cycle_cnt) 
begin 
	case spi_sm is 
		when IDLE => 
			if write_start = '1' then 
				if cycle_cnt <= 10 then
					led_data_next <= command_in; --"1011011101111001" for testing. 
					spi_sm_next <= OP_lo; 
				else 
					led_data_next <= command_in; --"1011011101111001" for testing. 
					spi_sm_next <= IDLE; 
				end if;
			else 
				led_data_next <= led_data; 
				spi_sm_next <= IDLE; 
			end if; 
			sm_cnt_next <= (others=>'0'); --state counter 
			led_clk_next <= '0';
			led_latch_next <= '0'; 
			cycle_cnt_next <= cycle_cnt; 
		when OP_lo => 
			led_data_next <= led_data; 
			spi_sm_next <= OP_hi;
			led_clk_next <= not led_clk; --toggle sclk
			sm_cnt_next <= sm_cnt;	
			led_latch_next <= '0';
			cycle_cnt_next <= cycle_cnt; 
		when OP_hi =>
			if sm_cnt>=15 then --state counter triggers at 15
				spi_sm_next <= LATCHRDY; 
				sm_cnt_next <= sm_cnt;
				led_latch_next <= '0'; 
			else 
				spi_sm_next <= OP_lo; 
				sm_cnt_next <= sm_cnt + 1; --sm counter increment
				led_latch_next <= '0';
			end if; 
			led_data_next(15 downto 1) <= led_data(14 downto 0); --shift the command out
			led_data_next(0) <= '0'; --pad '0';
			led_clk_next <= not led_clk; --toggle sclk
			cycle_cnt_next <= cycle_cnt; 
		when LATCHRDY => 
			led_data_next <= led_data; 
			spi_sm_next <= IDLE;
			led_clk_next <= '0'; --toggle sclk
			sm_cnt_next <= sm_cnt;	
			led_latch_next <= '1';
			cycle_cnt_next <= cycle_cnt + 1; 
	end case; 
end process; 


end Behavioral;

