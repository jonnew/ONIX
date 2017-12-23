--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;
library work;
use work.myDeclare.all;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY TB_async_com_control IS
END TB_async_com_control;
 
ARCHITECTURE behavior OF TB_async_com_control IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT async_com_control
    PORT(
         bus_clk : IN  std_logic;
         reset : IN  std_logic;
         
         conf_ack : IN  std_logic;
         conf_nack : IN  std_logic;
         conf_done : IN  std_logic;
			async_fifo_wr_enb : out std_logic; 
			async_fifo_wr_data : out std_logic_vector(7 downto 0)

        );
    END COMPONENT;
    

   --Inputs
   signal bus_clk : std_logic := '0';
   signal reset : std_logic := '0';
   signal hs_start : std_logic := '0';
   signal conf_ack : std_logic := '0';
   signal conf_nack : std_logic := '0';
   signal conf_done : std_logic := '0';

	signal async_fifo_wr_enb : std_logic; 
	signal async_fifo_wr_data : std_logic_vector(7 downto 0);

   -- Clock period definitions
   constant bus_clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: async_com_control PORT MAP (
          bus_clk => bus_clk,
          reset => reset,
          hs_start => hs_start,
          conf_ack => conf_ack,
          conf_nack => conf_nack,
          conf_done => conf_done,
			 async_fifo_wr_enb => async_fifo_wr_enb,
			 async_fifo_wr_data => async_fifo_wr_data
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
		reset <= '1'; 
      wait for 100 ns;	
		reset <= '0'; 

      wait for bus_clk_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
