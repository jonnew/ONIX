--------------------------------------------------------------------------------
--Test bench for the SPI_module
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY TB_SPI_module IS
END TB_SPI_module;
 
ARCHITECTURE behavior OF TB_SPI_module IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT SPI_module
    PORT(
         clk_spi : IN  std_logic;
         reset : IN  std_logic;
         spi_start : IN  std_logic;
			command_in : in std_logic_vector(15 downto 0);
			miso_i : in std_logic; 
         cs_o : OUT  std_logic;
         sclk_o : OUT  std_logic;
         mosi_o : OUT  std_logic;
			data_lclk_o : out std_logic
        );
    END COMPONENT;		 

   --Inputs
   signal clk_spi : std_logic := '0';
   signal reset : std_logic := '0';
   signal spi_start : std_logic := '0';
	signal command_in : std_logic_vector(15 downto 0);
	signal miso_i : std_logic; 

 	--Outputs
   signal cs_o : std_logic;
   signal sclk_o : std_logic;
   signal mosi_o : std_logic;
	signal data_lclk_o : std_logic;


   -- Clock period definitions
   constant clk_spi_period : time := 10 ns;
	constant spi_start_period : time := 1 us;
 
BEGIN
 
	-- output SPI module
   uut_output: SPI_module PORT MAP (
          clk_spi => clk_spi,
          reset => reset,
          spi_start => spi_start,
			 command_in => command_in,
			 miso_i => miso_i,
          cs_o => cs_o,
          sclk_o => sclk_o,
          mosi_o => mosi_o,
			 data_lclk_o => data_lclk_o
        );

	-- Instantiate the Unit Under Test (UUT)
   uut_input: SPI_module PORT MAP (
          clk_spi => clk_spi,
          reset => reset,
          spi_start => spi_start,
			 command_in => command_in,
			 miso_i => mosi_o,
          cs_o => open,
          sclk_o => open,
          mosi_o => open,
			 data_lclk_o => open
        );


   -- Clock process definitions
   clk_spi_process :process
   begin
		clk_spi <= '0';
		wait for clk_spi_period/2;
		clk_spi <= '1';
		wait for clk_spi_period/2;
   end process;
 
   spi_start_process :process
   begin
		spi_start <= '1';
		wait for spi_start_period;
		spi_start <= '1';
		wait for 40 ns;
   end process;

   -- Stimulus process
   stim_proc: process
   begin		
		command_in <= "1000000001011011";
		miso_i <= '1';
      -- hold reset state for 100 ns.
		reset <= '1'; 
      wait for 100 ns;
		reset <= '0'; 
		
      wait for clk_spi_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
