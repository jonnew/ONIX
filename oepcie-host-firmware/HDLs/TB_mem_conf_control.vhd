
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;
use work.myDeclare.all;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY TB_mem_conf_control IS
END TB_mem_conf_control;
 
ARCHITECTURE behavior OF TB_mem_conf_control IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT mem_conf_control
    PORT(
         bus_clk : IN  std_logic;
         reset : IN  std_logic;
         user_mem_32_addr : IN  std_logic_vector(2 downto 0);
         user_w_mem_32_wren : IN  std_logic;
         user_r_mem_32_rden : IN  std_logic;
         user_w_mem_32_data : IN  std_logic_vector(31 downto 0);
         user_r_mem_32_data : OUT  std_logic_vector(31 downto 0);
         dev_reset_out : out std_logic;
         conf_ack : OUT  std_logic;
         conf_nack : OUT  std_logic;
			mem_out : out mem_type 
        );
    END COMPONENT;
    
    COMPONENT async_com_control
    PORT(
         bus_clk : IN  std_logic;
         reset : IN  std_logic;
         dev_reset_in : in std_logic;
         conf_ack : IN  std_logic;
         conf_nack : IN  std_logic;
         conf_done : IN  std_logic;
			conf_mem_in : in mem_type; 
			async_fifo_wr_enb : out std_logic; 
			async_fifo_wr_data : out std_logic_vector(7 downto 0)

        );
    END COMPONENT;

   --Inputs
   signal bus_clk : std_logic := '0';
   signal reset : std_logic := '0';
   signal user_mem_32_addr : std_logic_vector(2 downto 0) := (others => '0');
   signal user_w_mem_32_wren : std_logic := '0';
   signal user_r_mem_32_rden : std_logic := '0';
   signal user_w_mem_32_data : std_logic_vector(31 downto 0) := (others => '0');

 	--Outputs
   signal user_r_mem_32_data : std_logic_vector(31 downto 0);
   signal conf_ack : std_logic;
   signal conf_nack : std_logic;
	
	signal hs_start : std_logic := '0';
   signal conf_done : std_logic := '0';
	signal async_fifo_wr_enb : std_logic; 
	signal async_fifo_wr_data : std_logic_vector(7 downto 0);
	
	signal conf_mem : mem_type;
	
	signal dev_reset : std_logic;

   -- Clock period definitions
   constant bus_clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: mem_conf_control PORT MAP (
          bus_clk => bus_clk,
          reset => reset,
          user_mem_32_addr => user_mem_32_addr,
          user_w_mem_32_wren => user_w_mem_32_wren,
          user_r_mem_32_rden => user_r_mem_32_rden,
          user_w_mem_32_data => user_w_mem_32_data,
          user_r_mem_32_data => user_r_mem_32_data,
          dev_reset_out => dev_reset,
          conf_ack => conf_ack,
          conf_nack => conf_nack,
			 mem_out => conf_mem
        );
		  
	uut2: async_com_control PORT MAP (
          bus_clk => bus_clk,
          reset => reset,
          dev_reset_in => dev_reset,
          conf_ack => conf_ack,
          conf_nack => conf_nack,
          conf_done => conf_done,
			 conf_mem_in => conf_mem,
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
		user_w_mem_32_wren <= '1';
		user_mem_32_addr <= std_logic_vector(to_unsigned(2, 3)); 
		user_w_mem_32_data <= std_logic_vector(to_unsigned(134, 32)); 
		
		
		reset <= '1'; 
      -- hold reset state for 100 ns.
      wait for 100 ns;	
		reset <= '0'; 
		
		user_w_mem_32_wren <= '1';
		user_mem_32_addr <= std_logic_vector(to_unsigned(2, 3)); 
		user_w_mem_32_data <= std_logic_vector(to_unsigned(134, 32)); 
		wait for 50 ns; 
		
		user_w_mem_32_wren <= '1';
		user_mem_32_addr <= std_logic_vector(to_unsigned(1, 3)); 
		user_w_mem_32_data <= std_logic_vector(to_unsigned(152, 32)); 
		wait for 50 ns; 
		
		user_w_mem_32_wren <= '1';
		user_mem_32_addr <= std_logic_vector(to_unsigned(4, 3)); 
		user_w_mem_32_data <= std_logic_vector(to_unsigned(1, 32)); 
		
      wait for bus_clk_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
