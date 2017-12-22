--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all; 
use work.myDeclare.all;
 
ENTITY TB_com_scheduler IS
END TB_com_scheduler;
 
ARCHITECTURE behavior OF TB_com_scheduler IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT com_scheduler
    PORT(
         clk : IN  std_logic;
         reset : IN  std_logic;
			device_data_array : in device_data_array_type; -- Array of dimension (No. of device x 16bits) data, each 16bits corresponds to 1 device input stream
			device_flag_array : in std_logic_vector(0 to NUMBEROFDEVICE-1);  
         serdes_data_out : OUT  std_logic_vector(11 downto 0);
         serdes_valid_out : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal clk : std_logic := '0';
   signal reset : std_logic := '0';
   signal device_data_array : device_data_array_type;
   signal device_flag_array : std_logic_vector(0 to NUMBEROFDEVICE-1); 

 	--Outputs
   signal serdes_data_out : std_logic_vector(11 downto 0);
   signal serdes_valid_out : std_logic;

   -- Clock period definitions
   constant clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: com_scheduler PORT MAP (
          clk => clk,
          reset => reset,
          device_data_array => device_data_array,
          device_flag_array => device_flag_array,
          serdes_data_out => serdes_data_out,
          serdes_valid_out => serdes_valid_out
        );

   -- Clock process definitions
   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
		-- insert stimulus here 
		device_data_array(0) <= std_logic_vector(to_unsigned(172, 16)); 
		device_data_array(1) <= std_logic_vector(to_unsigned(65, 16)); 
		device_data_array(2) <= std_logic_vector(to_unsigned(32, 16)); 
		device_data_array(3) <= std_logic_vector(to_unsigned(88, 16)); 
		device_flag_array <= (others=>'1');
      -- hold reset state for 100 ns.
		reset <= '1'; 
      wait for 100 ns;	
		reset <= '0'; 
      wait for clk_period*10;
		
		
      wait;
   end process;

END;
