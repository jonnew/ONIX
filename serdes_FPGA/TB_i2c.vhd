--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   14:26:31 05/25/2017
-- Design Name:   
-- Module Name:   C:/X/serdes_FPGA/TB_i2c.vhd
-- Project Name:  serdes_FPGA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: i2c_master
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;
 
ENTITY TB_i2c IS
END TB_i2c;
 
ARCHITECTURE behavior OF TB_i2c IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT i2c_master
	   GENERIC(
		 input_clk : INTEGER := 50_000_000; --input clock speed from user logic in Hz
		 bus_clk   : INTEGER := 400_000);   --speed the i2c bus (scl) will run at in Hz
    PORT(
         clk : IN  std_logic;
         reset_n : IN  std_logic;
         ena : IN  std_logic;
         addr : IN  std_logic_vector(6 downto 0);
         rw : IN  std_logic;
         data_wr : IN  std_logic_vector(7 downto 0);
         busy : OUT  std_logic;
         data_rd : OUT  std_logic_vector(7 downto 0);
         ack_error : BUFFER  std_logic;
         sda : INOUT  std_logic;
         scl : INOUT  std_logic
        );
    END COMPONENT;
	 
	 component I2CslaveWith8bitsIO
	 port (
		 SDA: inout std_logic;
		 SCL: in std_logic; 
		IOout: out std_logic_vector(7 downto 0)
		);
	end component;
    
	 
	 component i2cs_rx is
	generic(
		WR       : std_logic:='0';
		DADDR		: std_logic_vector(6 downto 0); --:= "0010001";		   -- 11h (22h) device address
		ADDR		: std_logic_vector(7 downto 0)  --:= "00000000"		   -- 00h	    sub address		
	);
	port(
		RST		: in std_logic;
		SCL		: in std_logic;
		SDA		: inout std_logic;
		DOUT 		: out std_logic_vector(7 downto 0)			   -- Recepted over i2c data byte
	);	end component;
	 

   --Inputs
   signal clk : std_logic := '0';
   signal reset_n : std_logic := '0';
   signal ena : std_logic := '0';
   signal addr : std_logic_vector(6 downto 0) := (others => '0');
   signal rw : std_logic := '0';
   signal data_wr : std_logic_vector(7 downto 0) := (others => '0');
	signal IOout : std_logic_vector(7 downto 0);

	--BiDirs
   signal sda : std_logic;
   signal scl : std_logic;

 	--Outputs
   signal busy : std_logic;
   signal data_rd : std_logic_vector(7 downto 0);
   signal ack_error : std_logic;

   -- Clock period definitions
   constant clk_period : time := 20 ns;
 
BEGIN
	-- Instantiate the Unit Under Test (UUT)
   uut: i2c_master generic map(input_clk=>50_000_000,
	bus_clk => 400_000
	)
	PORT MAP (
          clk => clk,
          reset_n => reset_n, --active low
          ena => ena,
          addr => addr,
          rw => rw,
          data_wr => data_wr,
          busy => busy,
          data_rd => data_rd,
          ack_error => ack_error,
          sda => sda,
          scl => scl
        );
		  
	i2c_slave: i2cs_rx generic map(WR=>'0',
	DADDR => "0000001",
	ADDR => "00000000")
	port map (
		RST => '0',
		SCL => scl, 
		SDA => sda,
		DOUT => IOout			   -- Recepted over i2c data byte
	); 

   -- Clock process definitions
   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 
   ena_process :process
   begin
		ena <= '0';
		wait for 1 us;
		ena <= '1';
		wait for 20 ns;
   end process;


   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
			sda <= 'H';
	scl <= 'H'; 
		addr <= "0000001"; 
		rw <= '0'; 
		data_wr <= "00101101"; 
		reset_n <= '0'; 
		
      wait for 100 ns;	
		reset_n <= '1'; 
      wait for clk_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
