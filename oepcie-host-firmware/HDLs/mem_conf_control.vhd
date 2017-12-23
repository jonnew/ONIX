----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use WORK.myDeclare.all;

entity mem_conf_control is
port (
	bus_clk : in std_logic; 
	reset : in std_logic; 
	user_mem_32_addr : in std_logic_vector(3 downto 0);
	user_w_mem_32_wren : in std_logic; 
	user_r_mem_32_rden : in std_logic; 
	user_w_mem_32_data : in std_logic_vector(31 downto 0);
	user_r_mem_32_data : out std_logic_vector(31 downto 0); 
	dev_reset_out : out std_logic; 
	conf_ack : out std_logic;
	conf_nack : out std_logic;
	mem_out : out mem_type
);
end mem_conf_control;

architecture Behavioral of mem_conf_control is

--state machines
type confstate_type is (MEMUD, CONF, ACK, NACK, DEVRESET); --state machine definition
signal confstate : confstate_type;  


--memory location and its functions; 
constant HS_CONFIG_DEVICE_ID : integer := 0; 
constant HS_CONFIG_REG_ADDR  : integer := 1; 
constant HS_CONFIG_REG_VALUE : integer := 2;  
constant HS_CONFIG_RW 		  : integer := 3; 
constant HS_CONFIG_TRIG      : integer := 4; 
constant KC705_RUNNING    : integer := 5; 
constant KC705_RESET      : integer := 6; 
constant KC705_SYS_CLK_HZ : integer := 7; 
constant KC705_FRAME_CLK_HZ : integer :=8; 
constant KC705_FRAME_CLK_M  : integer :=9; 
constant KC705_FRAME_CLK_D  : integer :=10;

signal mem_host : mem_type; 

begin

mem_out <= mem_host; 

sm_proc: process(bus_clk, reset, user_mem_32_addr, mem_host, user_w_mem_32_wren, user_r_mem_32_rden) 
begin 
	if (reset = '1') then 
		confstate <= MEMUD;
		conf_ack <= '0'; 
		conf_nack <= '0'; 
		dev_reset_out <= '0'; --reset device 
		for i in 0 to MEMARRAYLENGTH-1 loop 
			mem_host(i) <= (others=>'0');
		end loop; 
	elsif (rising_edge(bus_clk)) then 
		case confstate is 
			when MEMUD => 
			
				--update mem_host
				if (user_w_mem_32_wren = '1') then 
				    --if user_mem_32_addr = 
					mem_host(to_integer(unsigned(user_mem_32_addr))) <= user_w_mem_32_data;
				else --user update the read only registers
					mem_host(KC705_SYS_CLK_HZ) <= std_logic_vector(to_unsigned(250_000_000,32));
					mem_host(KC705_FRAME_CLK_HZ) <= std_logic_vector(to_unsigned(1000,32));
				end if;
				
				if (user_r_mem_32_rden = '1') then
					user_r_mem_32_data <= mem_host(to_integer(unsigned(user_mem_32_addr)));
				else 
					user_r_mem_32_data <= (others=>'0'); 
				end if; 
				
				if (not (mem_host(KC705_RESET) = (x"00000000"))) then 
					confstate <= DEVRESET;
				elsif (not (mem_host(HS_CONFIG_TRIG) = (x"00000000"))) then 
					confstate <= CONF;
				else 
					confstate <= MEMUD; 
				end if; 
				
				conf_nack <= '0'; 
				conf_ack <= '0'; 
				dev_reset_out <= '0'; 
			when CONF => 
				--here initiate configuration to the headstage. 
				--right now always ACK 
				confstate <= ACK;
			when ACK => 
				--send ACK signal 
				conf_ack <= '1'; 
				mem_host(HS_CONFIG_TRIG) <= (others=>'0'); --reset this to 0. 
				confstate <= MEMUD; 
			when NACK => 
			    conf_nack <= '1'; 
			    confstate <= MEMUD; 
			when DEVRESET => -- here we need to send a fresh device map to the host and set this register back to zero
				 dev_reset_out <= '1'; 
				 confstate <= MEMUD; 
				 mem_host(KC705_RESET) <= (others=>'0'); 
		end case; 
	end if; 
end process; 


end Behavioral;