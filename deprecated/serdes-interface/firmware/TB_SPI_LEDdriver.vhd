--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY TB_SPI_LEDdriver IS
END TB_SPI_LEDdriver;
 
ARCHITECTURE behavior OF TB_SPI_LEDdriver IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT SPI_LEDdriver
    PORT(
         clk_spi : IN  std_logic;
         reset : IN  std_logic;
         write_start : IN  std_logic;
         command_in : IN  std_logic_vector(15 downto 0);
         led_clk_o : OUT  std_logic;
         led_data_o : OUT  std_logic;
         led_latch_o : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal clk_spi : std_logic := '0';
   signal reset : std_logic := '0';
   signal write_start : std_logic := '0';
   signal command_in : std_logic_vector(15 downto 0) := (others => '0');

 	--Outputs
   signal led_clk_o : std_logic;
   signal led_data_o : std_logic;
   signal led_latch_o : std_logic;

   -- Clock period definitions
   constant clk_spi_period : time := 10 ns;
	constant write_start_period : time := 1 us;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: SPI_LEDdriver PORT MAP (
          clk_spi => clk_spi,
          reset => reset,
          write_start => write_start,
          command_in => command_in,
          led_clk_o => led_clk_o,
          led_data_o => led_data_o,
          led_latch_o => led_latch_o
        );

   -- Clock process definitions
   clk_spi_process :process
   begin
		clk_spi <= '0';
		wait for clk_spi_period/2;
		clk_spi <= '1';
		wait for clk_spi_period/2;
   end process;
 
   write_start_process :process
   begin
		write_start <= '0';
		wait for write_start_period;
		write_start <= '1';
		wait for 40 ns;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
		command_in <= "1000000001011011";
      -- hold reset state for 100 ns.
		reset <= '1'; 
      wait for 100 ns;	
		reset <= '0'; 
      wait for clk_spi_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
