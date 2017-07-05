----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    19:56:40 05/16/2017 
-- Design Name: 
-- Module Name:    data_split - Behavioral 
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

entity data_split is
port(
	pclk  : in std_logic; 
	reset : in std_logic; 
	vsync : in std_logic; 
	din   : in std_logic_vector(7 downto 0);
	stream1_o : out std_logic_vector(15 downto 0); 
	stream2_o : out std_logic_vector(15 downto 0); 
	stream3_o : out std_logic_vector(15 downto 0); 
	stream4_o : out std_logic_vector(15 downto 0);
	vsync_pcie_o : out std_logic
);
end data_split;

architecture Behavioral of data_split is
type split_state_type is (IDLE, S1MSB, S1LSB, S2MSB, S2LSB, S3MSB, S3LSB, S4MSB, S4LSB, LATCHDATA, WAITLOW); --state machine definition
signal split_state, split_state_next : split_state_type;  
signal stream1, stream1_next: std_logic_vector(15 downto 0); 
signal stream2, stream2_next: std_logic_vector(15 downto 0); 
signal stream3, stream3_next: std_logic_vector(15 downto 0); 
signal stream4, stream4_next: std_logic_vector(15 downto 0); 

signal stream1_masked : std_logic_vector(15 downto 0);
signal stream2_masked : std_logic_vector(15 downto 0);
signal stream3_masked : std_logic_vector(15 downto 0);
signal stream4_masked : std_logic_vector(15 downto 0);

signal vsync_pcie : std_logic; 

begin
--signal mapping 
vsync_pcie_o <= vsync_pcie;
stream1_o <= stream1_masked; 
stream2_o <= stream2_masked; 
stream3_o <= stream3_masked; 
stream4_o <= stream4_masked; 

--vsync triggers the data spliting process
process(reset, split_state, pclk, stream1, stream2, stream3, stream4, stream1_masked, stream2_masked, stream3_masked, stream3_masked)
begin 
	if (reset='1') then 
		split_state <= IDLE; 
		stream1 <= (others=>'0'); 
		stream2 <= (others=>'0'); 
		stream3 <= (others=>'0'); 
		stream4 <= (others=>'0'); 
		stream1_masked <= (others=>'0'); 
		stream2_masked <= (others=>'0'); 
		stream3_masked <= (others=>'0'); 
		stream4_masked <= (others=>'0'); 
		vsync_pcie <= '0'; 
	elsif (rising_edge(pclk)) then 
		split_state <= split_state_next; 
		stream1 <= stream1_next; 
		stream2 <= stream2_next; 
		stream3 <= stream3_next; 
		stream4 <= stream4_next; 
		
		if split_state = WAITLOW then 
			vsync_pcie <= '1'; 
		else 
			vsync_pcie <= '0'; 
		end if; 
		
		if split_state = LATCHDATA then 
			stream1_masked <= stream1; 
			stream2_masked <= stream2; 
			stream3_masked <= stream3; 
			stream4_masked <= stream4;
		else 
			stream1_masked <= stream1_masked; 
			stream2_masked <= stream2_masked; 
			stream3_masked <= stream3_masked; 
			stream4_masked <= stream4_masked;
		end if; 
	end if; 
end process; 

--next process
process(reset, split_state, vsync, pclk, stream1, stream2, stream3, stream4, din) 
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
			--2,3,4 unchanged
			stream2_next <= stream2;
			stream3_next <= stream3;
			stream4_next <= stream4;
		when S1MSB => 
			stream1_next(15 downto 8) <= stream1(15 downto 8); 
			stream1_next(7 downto 0) <= din;
			split_state_next <= S1LSB;
			--2,3,4 unchanged
			stream2_next <= stream2;
			stream3_next <= stream3;
			stream4_next <= stream4;
		when S1LSB => 
			split_state_next <= S2MSB;
			--2 MSB
			stream2_next(15 downto 8) <= din; 
			stream2_next(7 downto 0) <= stream2(7 downto 0);
			--1,3,4 unchanged
			stream1_next <= stream1; 
			stream3_next <= stream3;
			stream4_next <= stream4;
		when S2MSB => 
			split_state_next <= S2LSB;
			--2 LSB
			stream2_next(15 downto 8) <= stream2(15 downto 8); 
			stream2_next(7 downto 0) <= din;
			--1,3,4 unchanged
			stream1_next <= stream1; 
			stream3_next <= stream3;
			stream4_next <= stream4;			
		when S2LSB => 
			split_state_next <= S3MSB;
			--3 MSB
			stream3_next(15 downto 8) <= din; 
			stream3_next(7 downto 0) <= stream3(7 downto 0);
			--1,2,4 unchanged
			stream1_next <= stream1; 
			stream2_next <= stream2;
			stream4_next <= stream4;
		when S3MSB => 
			split_state_next <= S3LSB;
			--3 LSB
			stream3_next(15 downto 8) <= stream3(15 downto 8);  
			stream3_next(7 downto 0) <= din;
			--1,2,4 unchanged
			stream1_next <= stream1; 
			stream2_next <= stream2;
			stream4_next <= stream4;			
		when S3LSB => 
			split_state_next <= S4MSB;
			--4 MSB
			stream4_next(15 downto 8) <= din; 
			stream4_next(7 downto 0) <= stream4(7 downto 0);
			--1,2,3 unchanged
			stream1_next <= stream1; 
			stream2_next <= stream2;
			stream3_next <= stream3;	
		when S4MSB => 
			split_state_next <= S4LSB;
			--4 LSB
			stream4_next(15 downto 8) <= stream4(15 downto 8);  
			stream4_next(7 downto 0) <= din;
			--1,2,3 unchanged
			stream1_next <= stream1; 
			stream2_next <= stream2;
			stream3_next <= stream3;
		when S4LSB => 
			split_state_next <= LATCHDATA;
			stream1_next <= stream1; 
			stream2_next <= stream2;
			stream3_next <= stream3;
			stream4_next <= stream4;
		when LATCHDATA => 
			stream1_next <= stream1; 
			split_state_next <= WAITLOW;
			stream1_next <= stream1; 
			stream2_next <= stream2;
			stream3_next <= stream3;
			stream4_next <= stream4;
		when WAITLOW => 
			if (vsync = '0') then
				split_state_next <= IDLE;
			else
				split_state_next <= WAITLOW;
			end if; 
			stream1_next <= stream1; 
			stream2_next <= stream2;
			stream3_next <= stream3;
			stream4_next <= stream4;
	end case; 
end process;  

end Behavioral;

