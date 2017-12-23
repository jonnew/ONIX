--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;
use work.myDeclare.all;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY TB_hs_com_control IS
END TB_hs_com_control;
 
ARCHITECTURE behavior OF TB_hs_com_control IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT hs_com_control
    PORT(
         bus_clk : IN  std_logic;
         global_reset : IN  std_logic;
         hs_com_fifo_data : OUT  std_logic_vector(31 downto 0);
			dev_reset_in : in std_logic; 
         hs_com_fifo_enb : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal bus_clk : std_logic := '0';
   signal global_reset : std_logic := '0';
	signal dev_reset_in : std_logic := '0'; 

 	--Outputs
   signal hs_com_fifo_data : std_logic_vector(31 downto 0);
   signal hs_com_fifo_enb : std_logic;

   -- Clock period definitions
   constant bus_clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: hs_com_control PORT MAP (
          bus_clk => bus_clk,
          global_reset => global_reset,
			 dev_reset_in => dev_reset_in,
          hs_com_fifo_data => hs_com_fifo_data,
          hs_com_fifo_enb => hs_com_fifo_enb
        );

   -- Clock process definitions
   bus_clk_process :process
   begin
		bus_clk <= '0';
		wait for bus_clk_period/2;
		bus_clk <= '1';
		wait for bus_clk_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
		global_reset <= '1'; 
		dev_reset_in <= '1';
      wait for 100 ns;	
		global_reset <= '0'; 
		dev_reset_in <= '0';
      
      wait for 5 ms;
      global_reset <= '1'; 
      wait for 100 ns; 
      global_reset <= '0';

      wait;
   end process;

END;
