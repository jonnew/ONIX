<<<<<<< HEAD
--Test bench for i2c interface
--by: Jie (Jack) Zhang MWL-MIT

=======
--------------------------------------------------------------------------------
>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;
 
<<<<<<< HEAD
=======
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c
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
<<<<<<< HEAD
			rd_from_remote : in std_logic; 
=======
>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c
         sda : INOUT  std_logic;
         scl : INOUT  std_logic
        );
    END COMPONENT;
	 
	 --slave device
	 component i2c_slave is
	  generic(
			input_clk : INTEGER := 50_000_000; --input clock speed from user logic in Hz
<<<<<<< HEAD
			bus_clk   : INTEGER := 500_000;   --speed the i2c bus (scl) will run at in Hz
			ID        : std_logic_vector(6 downto 0) := "1010101"); --Device specific ID
=======
			bus_clk   : INTEGER := 500_000);   --speed the i2c bus (scl) will run at in Hz
>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c
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
    
<<<<<<< HEAD
	 component i2c_master_init is
		port(
				clk 	: in std_logic; 	--same clock for the i2c interface	
				reset : in std_logic; 
				busy  : in std_logic;
				ack_error : in std_logic;
				i2c_ena_o : out std_logic;
				rw_o      : out std_logic; 
				device_id_o : out std_logic_vector(6 downto 0);
				addr_o  : out std_logic_vector(7 downto 0); 
				value_o : out std_logic_vector(7 downto 0);
				user_rw : in std_logic; 
				user_device_id : in std_logic_vector(6 downto 0); 
				user_addr : in std_logic_vector(7 downto 0);
				user_value : in std_logic_vector(7 downto 0)
		);
	 end component;
=======
>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c

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
<<<<<<< HEAD
			 rd_from_remote => '0',
=======
>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c
          sda => sda,
          scl => scl
        );

	-- slave module 
<<<<<<< HEAD
	uut_i2c_slave_des : i2c_slave generic map (
			ID => "1100000"
	)port map (		
			clk      => clk,
			reset    => reset,                       --active high reset
			sda      => sda,                   --serial data i2c bus
			scl      => scl, 						 --serial clock i2c bus
			wr_enb   => wr_enb,   
			rd_enb   => rd_enb,  
			addrout  => open,  
			regin    => "10111001", 
			regout   => open              
			); 
    

	uut_i2c_slave_ser : i2c_slave generic map (
			ID => "1011000"
	)port map (		
			clk      => clk,
			reset    => reset,                       --active high reset
			sda      => sda,                   --serial data i2c bus
			scl      => scl, 						 --serial clock i2c bus
			wr_enb   => wr_enb,   
			rd_enb   => rd_enb,  
			addrout  => open,  
			regin    => regin, 
			regout   => open              
			); 
			
	uut_i2c_slave_remote : i2c_slave generic map (
			ID => "1010000"
	)port map (		
=======
	uut_i2c_slave : i2c_slave port map (		
>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c
			clk      => clk,
			reset    => reset,                       --active high reset
			sda      => sda,                   --serial data i2c bus
			scl      => scl, 						 --serial clock i2c bus
			wr_enb   => wr_enb,   
			rd_enb   => rd_enb,  
			addrout  => addrout,  
<<<<<<< HEAD
			regin    => "10111001", 
			regout   => regout              
			); 

	control_init : i2c_master_init port map(
			clk 	=> clk, 	
			reset => reset,  
			busy  => busy,
			ack_error => ack_error, 
			i2c_ena_o => ena,
			rw_o => rw,
			device_id_o => devid,
			addr_o => addr,
			value_o => data_wr,
			user_rw => '1',
			user_device_id => "1010000", 
			user_addr => "00110110", 
			user_value => "01110100"
	);

=======
			regin    => regin, 
			regout   => regout              
			); 
    
>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c

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
<<<<<<< HEAD
      wait for 100 ns;
		reset <= '0'; 
		regin <= "10101100";
      wait for clk_period*10;
		
		wait for clk_period*100;
      -- insert stimulus here 
		wait for 200 us; 
		scl <= '0';
		wait for 200 us; 
		scl <= 'H';
=======
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

>>>>>>> 9e62c29c2a11e27a321e2c4a2c9d40dc76aee79c
      wait;
   end process;

END;
