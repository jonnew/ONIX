----------------------------------------------------------------------------------
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

---- Uncomment the following library declaration if instantiating
---- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity clk_div is
	generic (MAXD: natural:=5);
	port(
		 clk: in std_logic;
		 reset: in std_logic;
		 div: in integer range 0 to MAXD;
		 div_clk: out std_logic 
		 );
end clk_div;


architecture Behavioral of clk_div is

begin
process(clk,reset)
variable M: integer range 0 to MAXD;
begin
 if reset='1' then --reset clock divider 
	M := 0;
	div_clk <= '0';
 elsif(rising_edge(clk)) then -- generate a pulse when the counter = (the division magnitude -1)  
	if M=div-1 then
		div_clk <= '1';
		M := 0;
	else
		M := M +1 ;
		div_clk <= '0';
	end if;
 end if;
end process;
end Behavioral; 

