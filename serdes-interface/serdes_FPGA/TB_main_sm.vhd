--------------------------------------------------------------------------------
--test bench for the main state machine
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;
 
ENTITY TB_main_sm IS
END TB_main_sm;
 
ARCHITECTURE behavior OF TB_main_sm IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
    COMPONENT main_sm
    PORT(
         clk_spi : IN  std_logic;
         reset : IN  std_logic;
         miso_reg : IN  std_logic_vector(15 downto 0);
         data_lclkin : IN  std_logic;
         spi_start_o : OUT  std_logic;
         command_o : OUT  std_logic_vector(15 downto 0);
			hsync_o : out std_logic
        );
    END COMPONENT;
	 
	 --SPI data merger unit 
	 component data_merge is
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
	 end component;
	 
	 --FIFO module 
	 COMPONENT fifo_test
	  PORT (
		 rst : IN STD_LOGIC;
		 wr_clk : IN STD_LOGIC;
		 rd_clk : IN STD_LOGIC;
		 din : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		 wr_en : IN STD_LOGIC;
		 rd_en : IN STD_LOGIC;
		 dout : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		 full : OUT STD_LOGIC;
		 empty : OUT STD_LOGIC
	  );
	END COMPONENT;
	 
	 --SPI data split 
	 component data_split is
		port(
			--dataclk : in std_logic; --100MHz clk
			pclk : in std_logic; 
			reset : in std_logic; 
			vsync : in std_logic; 
			din   : in std_logic_vector(7 downto 0);
			stream1_o : out std_logic_vector(15 downto 0); --this is the output to write to fifo
			stream2_o : out std_logic_vector(15 downto 0); 
			stream3_o : out std_logic_vector(15 downto 0); 
			stream4_o : out std_logic_vector(15 downto 0);
			vsync_pcie_o : out std_logic
		);
		end component;
    
	 --SPI module 
	 component SPI_module is
	 port(
		clk_spi : in std_logic; --spi clock from toplevel 
		reset : in std_logic;   --reset 
		spi_start : in std_logic; --spi initiate 
		command_in : in std_logic_vector(15 downto 0); --parallel command input vector
		--SPI inputs
		miso_i : in std_logic; 
		--SPI outputs  
		cs_o : out std_logic;   
		sclk_o : out std_logic; --sclk is always 2x slower than clk_spi
		mosi_o : out std_logic;
		--data latch clock 
		data_lclk_o : out std_logic;
		data_rdy_pcie_o : out std_logic; 
		data_pcie_A_o : out std_logic_vector(15 downto 0);
		data_pcie_B_o : out std_logic_vector(15 downto 0); 
		miso_reg_A_o : out std_logic_vector(15 downto 0);
		miso_reg_B_o : out std_logic_vector(15 downto 0)
	 );
	 end component;

   --Inputs
   signal clk_spi : std_logic := '0';
   signal reset : std_logic := '0';
   signal miso_reg_A, miso_reg_B : std_logic_vector(15 downto 0) := (others => '0');
   signal data_lclkin : std_logic := '0';
	signal miso : std_logic := '1';
	signal clk84M : std_logic := '0'; 
	
	 --Outputs
   signal spi_start_o : std_logic;
	signal data_rdy_pcie_o : std_logic; 
	signal data_pcie_A_o, data_pcie_B_o : std_logic_vector(15 downto 0);
   signal command_o : std_logic_vector(15 downto 0);
	signal cs, sclk, mosi : std_logic; 
	signal dout_o : std_logic_vector(7 downto 0); 
	signal vsync_o : std_logic; 
	signal pclk : std_logic; 
	signal stream1, stream2, stream3, stream4 : std_logic_vector(15 downto 0); 
	signal hsync_o : std_logic; 
	signal vsync_pcie_o : std_logic; 
	
	signal fifo_in : std_logic_vector(15 downto 0);  
	signal fifo_wr_enb_o : std_logic; 
	signal fifo_rd_enb_o : std_logic; 
	signal fifo_out_o : std_logic_vector(15 downto 0); --output to the main dataclk statemachine on pci
	
	
   -- Clock period definitions
   --constant clk_spi_period : time := 11.90476 ns;
	constant clk84M_period : time := 11.90476 ns; 
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   mainstatement: main_sm PORT MAP (
          clk_spi => clk_spi,
          reset => reset,
          miso_reg => miso_reg_A,
          data_lclkin => data_lclkin,
          spi_start_o => spi_start_o,
          command_o => command_o,
			 hsync_o => hsync_o
        );
		  
	--SPI data merger unit 
	merge: data_merge port map (
		pclk  => clk_spi, 
		reset => reset,
		data_rdy_pcie => data_rdy_pcie_o, --this is generated from the SPI interface. Here we must sample this line using 50MHz clock	
		vsync_o => vsync_o,  
		stream1 =>  data_pcie_A_o,
		stream2 =>  data_pcie_B_o, 
		stream3 => "0011110101010101",
		stream4 => "1100000001111110", 
		dout_o  => dout_o
	 );
	 
--	fifo_block: fifo_test
--	  PORT map(
--		 rst => reset, 
--		 wr_clk => pclk,
--		 rd_clk => clk84M,
--		 din => stream1,
--		 wr_en => fifo_wr_enb_o, 
--		 rd_en => fifo_rd_enb_o,
--		 dout => fifo_in,
--		 full => open, 
--		 empty => open); 
	 
	 --SPI data split unit 
	spliter: data_split port map (
		--dataclk => clk84M, 
		pclk => clk_spi, --2 times slower than clk84M
		reset => reset,  
		vsync => vsync_o, 
		din => dout_o, 
		--fifo_in => fifo_in, 
		--fifo_wr_enb_o => fifo_wr_enb_o,
		--fifo_rd_enb_o => fifo_rd_enb_o,
		--fifo_out_o => fifo_out_o, --output to the main dataclk statemachine on pci
		stream1_o => stream1,
		stream2_o => stream2,
		stream3_o => stream3, 
		stream4_o => stream4,
		vsync_pcie_o => vsync_pcie_o
	); 
	
   spimodule: SPI_module  PORT MAP (
				clk_spi => clk_spi, 
				reset => reset,
				spi_start => spi_start_o,
				command_in => command_o, --read from 40 to 44 registers
				--SPI inputs
				miso_i => miso,
				--SPI outputs  
				cs_o => cs,    
				sclk_o => sclk, --sclk is always 2x slower than clk_spi
				mosi_o => mosi,
				--data latch clock 
				data_lclk_o => data_lclkin,
				data_rdy_pcie_o => data_rdy_pcie_o,
				data_pcie_A_o => data_pcie_A_o,
				data_pcie_B_o => data_pcie_B_o,
				miso_reg_A_o => miso_reg_A,
				miso_reg_B_o => miso_reg_B
        );

   -- Clock process definitions
   clk_spi_process :process
   begin
		clk_spi <= '0';
		wait for clk84M_period;
		clk_spi <= '1';
		wait for clk84M_period;
   end process;
	
	   -- Clock process definitions
   clk84M_process :process
   begin
		clk84M <= '0';
		wait for clk84M_period/2;
		clk84M <= '1';
		wait for clk84M_period/2;
   end process;

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
		reset <= '1'; 
      wait for 100 ns;	
		reset <= '0'; 
      wait for clk84M_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
