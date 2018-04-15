----------------------------------------------------------------------------------
--this merges the data from different streams onto the serdes interface 
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all; 

entity data_merge is
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
end data_merge;

architecture Behavioral of data_merge is

signal pclk_data_rdy_pcie : std_logic; --pclk synced data_rdy_pcie signal
signal dout, dout_next : std_logic_vector(7 downto 0); --digital output 
signal vsync, vsync_next : std_logic;
type merge_state_type is (IDLE, S1MSB, S1LSB, S2MSB, S2LSB, S3MSB, S3LSB, S4MSB, S4LSB, WAITLOW); --state machine definition: 
signal merge_state, merge_state_next : merge_state_type;  
signal sm_cnt, sm_cnt_next : unsigned(3 downto 0); 

begin
--signal assignment 
vsync_o <= vsync; 
dout_o <= dout; 

--vsync triggers the data spliting process
process(reset, merge_state, pclk)
begin 
	if (reset='1') then 
		merge_state <= IDLE; 
		dout <= (others=>'0');
		vsync <= '0'; 
		sm_cnt <= (others=>'0'); 
	elsif (rising_edge(pclk)) then 
		merge_state <= merge_state_next; 
		dout <= dout_next;
		vsync <= vsync_next; 
		sm_cnt <= sm_cnt_next; 
	end if; 
end process; 

--next states 
process(reset, merge_state, data_rdy_pcie, sm_cnt, dout, stream1, stream2, stream3, stream4) 
begin 
	case merge_state is 
		when IDLE => 
			if data_rdy_pcie = '1' then 
				merge_state_next <= S1MSB; 
			else 
				merge_state_next <= IDLE;
			end if; 
			dout_next <= dout; 
			vsync_next <= '0'; 
			sm_cnt_next <= (others=>'0'); 
		when S1MSB => 
			merge_state_next <= S1LSB;
			dout_next <= stream1(15 downto 8); 
			vsync_next <= '1'; 
			sm_cnt_next <= (others=>'0'); 
		when S1LSB => 
			merge_state_next <= S2MSB;			
			dout_next <= stream1(7 downto 0);
			vsync_next <= '1'; 			
			sm_cnt_next <= (others=>'0'); 
		when S2MSB => 
			merge_state_next <= S2LSB;
			dout_next <= stream2(15 downto 8); 
			vsync_next <= '1'; 
			sm_cnt_next <= (others=>'0'); 
		when S2LSB => 
			merge_state_next <= S3MSB;			
			dout_next <= stream2(7 downto 0);
			vsync_next <= '1'; 			
			sm_cnt_next <= (others=>'0'); 
		when S3MSB => 
			merge_state_next <= S3LSB;
			dout_next <= stream3(15 downto 8); 
			vsync_next <= '1'; 
			sm_cnt_next <= (others=>'0'); 
		when S3LSB => 
			merge_state_next <= S4MSB;			
			dout_next <= stream3(7 downto 0);
			vsync_next <= '1'; 			
			sm_cnt_next <= (others=>'0'); 
		when S4MSB => 
			merge_state_next <= S4LSB;
			dout_next <= stream4(15 downto 8); 
			vsync_next <= '1'; 
			sm_cnt_next <= (others=>'0'); 
		when S4LSB => 
			merge_state_next <= WAITLOW;			
			dout_next <= stream4(7 downto 0);
			vsync_next <= '1'; 			
			sm_cnt_next <= (others=>'0'); 
		when WAITLOW => 
			if data_rdy_pcie = '0' then
				if sm_cnt >= 10 then 
					merge_state_next <= IDLE; 
					vsync_next <= '0'; 
					sm_cnt_next <= (others=>'0'); 
				else 
					sm_cnt_next <= sm_cnt + 1; 
					vsync_next <= '1'; 
					merge_state_next <= WAITLOW; 
				end if; 
			else 
				merge_state_next <= WAITLOW;
				vsync_next <= '1'; 
				sm_cnt_next <= sm_cnt;
			end if; 
			dout_next <= (others=>'0'); 
			--sm_cnt_next <= (others=>'0'); 
	end case; 
end process; 

end Behavioral;

