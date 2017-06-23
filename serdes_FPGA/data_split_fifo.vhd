----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    19:56:40 05/16/2017 
-- Design Name: 
-- Module Name:    data_split_fifo - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity data_split_fifo is
port(
	pclk  : in std_logic; --42 MHz
	dataclk : in std_logic; --84 MHz
	reset : in std_logic; 
	vsync : in std_logic; 
	din   : in std_logic_vector(7 downto 0);
	fifo_in : in std_logic_vector(15 downto 0);
	fifo_wr_enb_o : out std_logic;
	fifo_rd_enb_o : out std_logic; 
	fifo_out_o : out std_logic_vector(15 downto 0); 
	stream1_o : out std_logic_vector(15 downto 0) 
);
end data_split_fifo;

architecture Behavioral of data_split_fifo is

type split_state_type is (IDLE, S1MSB, S1LSB, FIFOWRITE, WAITLOW); --state machine definition
signal split_state, split_state_next : split_state_type;  

type read_state_type is (IDLE, READING, WAITLOW); --state machine definition
signal read_state, read_state_next : read_state_type;

signal stream1, stream1_next: std_logic_vector(15 downto 0); 
signal stream1_masked, stream1_masked_next : std_logic_vector(15 downto 0);
signal fifo_wr_enb, fifo_wr_enb_next : std_logic; 
signal fifo_rd_start, fifo_rd_start_next : std_logic; 
signal fifo_rd_enb, fifo_rd_enb_next: std_logic; 
signal fifo_out, fifo_out_next : std_logic_vector(15 downto 0); 

begin
--signal mapping 
stream1_o <= stream1_masked; 
fifo_out_o <= fifo_out; 
fifo_rd_enb_o <= fifo_rd_enb;
fifo_wr_enb_o <= fifo_wr_enb;

--fifo reading process (using fifo_rd_start to begin the fifo reading process) 
process(reset, dataclk) 
begin 
	if (reset='1') then 
		fifo_rd_enb <= '0';
		fifo_out <= (others=>'0'); 
		read_state <= IDLE;
	elsif (rising_edge(dataclk)) then 
		fifo_rd_enb <= fifo_rd_enb_next;
		fifo_out <= fifo_out_next; 
		read_state <= read_state_next; 
	end if; 
end process; 

--next process
process(reset, fifo_rd_start, fifo_in, read_state, fifo_out) 
begin 
	case read_state is 
		when IDLE => 
			if fifo_rd_start = '1' then 
				fifo_rd_enb_next <= '1'; 
				read_state_next <= READING; 
			else 
				fifo_rd_enb_next <= '0'; 
				read_state_next <= IDLE; 				
			end if; 
			fifo_out_next <= fifo_out; 
		when READING => 
			fifo_out_next <= fifo_in; 
			read_state_next <= WAITLOW; 	
			fifo_rd_enb_next <= '0'; 
		when WAITLOW => 
			if fifo_rd_start = '0' then 
				read_state_next <= IDLE;
			else 
				read_state_next <= WAITLOW;
			end if; 
			fifo_out_next <= fifo_out; 
			fifo_rd_enb_next <= '0'; 					
	end case; 
end process; 

--vsync triggers the data spliting process
process(reset, split_state, pclk)
begin 
	if (reset='1') then 
		split_state <= IDLE; 
		stream1 <= (others=>'0'); 
		stream1_masked <= (others=>'0'); 
		fifo_wr_enb <= '0'; 
		fifo_rd_start <= '0'; 
	elsif (rising_edge(pclk)) then 
		split_state <= split_state_next; 
		stream1 <= stream1_next; 
		stream1_masked <= stream1_masked_next; 
		fifo_wr_enb <= fifo_wr_enb_next; 
		fifo_rd_start <= fifo_rd_start_next; 
	end if; 
end process; 

--next process
process(reset, split_state, vsync, pclk, stream1, din, stream1_masked) 
begin 
	case split_state is 
		when IDLE => 
			if (vsync = '1') then 
				split_state_next <= S1MSB; 
				stream1_next(15 downto 8) <= din; 
				stream1_next(7 downto 0) <= stream1(7 downto 0); 
			else 
				split_state_next <= IDLE; 
				stream1_next <= stream1; 
			end if;
			stream1_masked_next <= stream1_masked;
			fifo_wr_enb_next <= '0'; 
			fifo_rd_start_next <= '0'; 
		when S1MSB => 
			stream1_next(15 downto 8) <= stream1(15 downto 8); 
			stream1_next(7 downto 0) <= din;
			stream1_masked_next <= stream1_masked;
			split_state_next <= S1LSB;
			fifo_wr_enb_next <= '0'; 
			fifo_rd_start_next <= '0'; 
		when S1LSB => 
			stream1_next <= stream1; 
			stream1_masked_next <= stream1;			
			split_state_next <= FIFOWRITE;
			fifo_wr_enb_next <= '1'; 
			fifo_rd_start_next <= '0'; 
		when FIFOWRITE => 
			stream1_next <= stream1; 
			stream1_masked_next <= stream1; 
			split_state_next <= WAITLOW;
			fifo_wr_enb_next <= '0'; 
			fifo_rd_start_next <= '1'; 
		when WAITLOW => 
			if (vsync = '0') then
				split_state_next <= IDLE;
			else
				split_state_next <= WAITLOW;
			end if; 
			stream1_next <= stream1; 
			stream1_masked_next <= stream1_masked;
			fifo_wr_enb_next <= '0'; 
			fifo_rd_start_next <= '0'; 
	end case; 
end process;  

end Behavioral;

