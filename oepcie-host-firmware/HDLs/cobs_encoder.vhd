--this is the COBS encoder

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;
library work;
use work.myDeclare.all;

entity cobs_encoder is
Port ( 
    bus_clk : in std_logic; 
    reset : in std_logic; 
	 --cobs inputs
    pre_cobs_data_in : in async_stream_type;
    data_in_length : in std_logic_vector(4 downto 0);
	 cobs_conv_begin : in std_logic;
	 --cobs outputs
    cobs_data_out : out cobs_stream_types;
    data_out_length : out std_logic_vector(4 downto 0);
	 cobs_conv_rdy : out std_logic
);
end cobs_encoder;

architecture Behavioral of cobs_encoder is

type cobs_sm_type is (IDLE, CONV);
signal cobs_sm : cobs_sm_type; 
signal vec_cnt : unsigned(4 downto 0);
signal idxreg : unsigned(4 downto 0);
signal cobs_data : cobs_stream_types;
signal pre_cobs_data : cobs_stream_types; --this is put into cobs_data format with the padding for consistency. 

begin

cobs_data_out <= cobs_data; 

--cobs process
cobs_proc: process(bus_clk, reset, cobs_conv_begin, idxreg, vec_cnt) 
begin 
	if (reset = '1') then 
		cobs_sm <= IDLE; 
		vec_cnt <= (others=>'0'); 
		idxreg <= to_unsigned(24,5); --always initialized to 254, last position of the COBS data.  
		cobs_conv_rdy <= '0'; 
		for i in 0 to 25 loop 
			cobs_data(i) <= (others=>'0'); 
			pre_cobs_data(i) <= (others=>'0'); 
		end loop;
	elsif (rising_edge(bus_clk)) then 
		case cobs_sm is
			when IDLE => --idle state
				if cobs_conv_begin = '1' then 
					cobs_sm <= CONV; 
					vec_cnt <= unsigned(data_in_length); --initilize the vector count to data_in_length 
					idxreg <= unsigned(data_in_length)+1; --initlize reg1 to data_in_length
					for i in 0 to 23 loop 
						pre_cobs_data(i+1) <= pre_cobs_data_in(i);
					end loop;
					pre_cobs_data(25) <= (others=>'0'); --always fill the 255 position with 0. 
					pre_cobs_data(0) <= (others=>'1');
					data_out_length <= (others=>'0'); 
				end if; 
				cobs_conv_rdy <= '0'; --lower the conv flag
			when CONV => 
				if vec_cnt >= 1 then 
					vec_cnt <= vec_cnt - 1; 
					if pre_cobs_data(to_integer(vec_cnt)) = "00000000" then 
						cobs_data(to_integer(vec_cnt)) <= "000" & std_logic_vector(idxreg - vec_cnt);
						idxreg <= vec_cnt; 
					else 
						cobs_data(to_integer(vec_cnt)) <= pre_cobs_data(to_integer(vec_cnt));
					end if; 
				else 
					cobs_sm <= IDLE; 
					cobs_data(0) <= "000" & std_logic_vector(idxreg);
					cobs_conv_rdy <= '1';
					data_out_length <= std_logic_vector(unsigned(data_in_length) + 2); 
				end if; 
		end case; 
	end if; 
end process; 

end Behavioral;
