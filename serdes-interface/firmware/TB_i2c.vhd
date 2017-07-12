--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY TB_i2c IS
END TB_i2c;
 
ARCHITECTURE behavior OF TB_i2c IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT i2c_master
	 generic(
			input_clk : INTEGER := 50_000_000; --input clock speed from user logic in Hz
			bus_clk   : INTEGER := 500_000);   --speed the i2c bus (scl) will run at in Hz
    port(
         clk : IN  std_logic;
         reset : IN  std_logic;
         ena : IN  std_logic;
			devid     : IN     STD_LOGIC_VECTOR(6 DOWNTO 0); --device id of target slave
         addr : IN  std_logic_vector(7 downto 0);
         rw : IN  std_logic;
         data_wr : IN  std_logic_vector(7 downto 0);
         busy : OUT  std_logic;
         data_rd : OUT  std_logic_vector(7 downto 0);
         ack_error : BUFFER  std_logic;
         sda : INOUT  std_logic;
         scl : INOUT  std_logic
        );
    END COMPONENT;
	 
	 --slave device
	 component i2c_slave is
	  generic(
			input_clk : INTEGER := 50_000_000; --input clock speed from user logic in Hz
			bus_clk   : INTEGER := 500_000);   --speed the i2c bus (scl) will run at in Hz
		port (
			clk       : IN     STD_LOGIC;                    --system clock
			reset   : IN     STD_LOGIC;                      --active high reset
			sda       : INOUT  STD_LOGIC;                    --serial data i2c bus
			scl       : INOUT  STD_LOGIC; 						 --serial clock i2c bus
			wr_enb    : out std_logic; --0: write to slave 1: read from slave
			rd_enb    : out std_logic; 
			addrout   : out std_logic_vector(7 downto 0);  
			regin     : in std_logic_vector(7 downto 0); --register values to send through i2c
			regout    : out std_logic_vector(7 downto 0)                  
			); 
		end component;
    

   --Inputs
   signal clk : std_logic := '0';
   signal reset : std_logic := '0';
   signal ena : std_logic := '0';
   signal devid : std_logic_vector(6 downto 0) := (others => '0');
	signal addr : std_logic_vector(7 downto 0) := (others => '0'); 
   signal rw : std_logic := '0';
   signal data_wr : std_logic_vector(7 downto 0) := (others => '0');
	signal 	wr_enb    :  std_logic; --0: write to slave 1: read from slave
	signal 	rd_enb    :  std_logic; 
	signal 	addrout   :  std_logic_vector(7 downto 0);  
	signal	regin     :  std_logic_vector(7 downto 0); --register values to send through i2c
	signal	regout    :  std_logic_vector(7 downto 0);   
	

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
   uut_i2c_master: i2c_master PORT MAP (
          clk => clk,
          reset => reset,
          ena => ena,
			 devid => devid,
          addr => addr,
          rw => rw,
          data_wr => data_wr,
          busy => busy,
          data_rd => data_rd,
          ack_error => ack_error,
          sda => sda,
          scl => scl
        );

	-- slave module 
	uut_i2c_slave : i2c_slave port map (		
			clk      => clk,
			reset    => reset,                       --active high reset
			sda      => sda,                   --serial data i2c bus
			scl      => scl, 						 --serial clock i2c bus
			wr_enb   => wr_enb,   
			rd_enb   => rd_enb,  
			addrout  => addrout,  
			regin    => regin, 
			regout   => regout              
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
      -- hold reset state for 100 ns.
		scl <= 'H'; 
		sda <= 'H';
		reset <= '1'; 
		addr <= "11001011";
		devid <= "1010101";
		data_wr <= "01011100";
		ena <= '0';
      wait for 100 ns;
		reset <= '0'; 
      wait for clk_period*10;
		ena <= '1'; 
		wait for clk_period*100;
		ena <= '0'; 
      -- insert stimulus here 

      wait;
   end process;

END;
