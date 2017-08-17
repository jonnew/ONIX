----------------------------------------------------------------------------------
-- This slave I2C interface
-- this slave module does not stretch the clock, because it doesn't need to.
-- by: Jie (Jack) Zhang MWL-MIT
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity i2c_slave is
	generic (
		input_clk : integer := 10_000_000; --input clock speed from user logic in Hz
		bus_clk : integer := 100_000; --speed the i2c bus (scl) will run at in Hz
	ID : std_logic_vector(6 downto 0) := "1010000"); --Device specific ID
	port (
		clk      : in std_logic; --system clock
		reset    : in std_logic; --active high reset
		sda      : inout std_logic; --serial data i2c bus
		scl      : inout std_logic; --serial clock i2c bus
		wr_enb   : out std_logic; --0: write to slave 1: read from slave
		rd_enb   : out std_logic;
		addrout  : out std_logic_vector(7 downto 0);
		regin    : in std_logic_vector(7 downto 0); --register values to send through i2c
		regout   : out std_logic_vector(7 downto 0)
	);
end i2c_slave;
 
architecture Behavioral of i2c_slave is

	signal clk10x : std_logic;
	signal sda_sync, scl_sync, sda_sync_dl, scl_sync_dl : std_logic;
	signal rx_cnt, rx_cnt_next : unsigned(3 downto 0);
	signal tx_cnt, tx_cnt_next : unsigned(2 downto 0); 
	signal data_reg, data_reg_next : std_logic_vector(7 downto 0);
	signal wr_reg, wr_reg_next : std_logic_vector(7 downto 0);
	signal rd_reg, rd_reg_next : std_logic_vector(7 downto 0);
	signal addr_reg, addr_reg_next : std_logic_vector(7 downto 0);
	signal sda_i, sda_i_next : std_logic;
	signal wr_rd, wr_rd_next : std_logic;
	signal datacnt, datacnt_next : std_logic;

	constant divider : integer := (input_clk/bus_clk)/10; --number of clocks in 1/10 cycle of scl
	type machine is(READY, DEVICEID, SLV_ACK1, WRVALUE, SLV_ACK2, RDVALUE, PRESTOP, STOP); --needed states
	signal slv_state, slv_state_next : machine;

	--a general clock divider
	component clk_div is
		generic (MAXD : natural := 5);
		port (
			clk      : in std_logic;
			reset    : in std_logic;
			div      : in integer range 0 to MAXD;
			div_clk  : out std_logic
		);
	end component;

begin
	--mapping
	addrout <= addr_reg(7 downto 0);
	regout <= wr_reg(7 downto 0);

	--sync sda and scl inout pins and delay them for 1 clock cycle
	i2c_sync_proc : process (clk10x, reset)
	begin
		if (reset = '1') then
			sda_sync <= '0';
			scl_sync <= '0';
			sda_sync_dl <= '0';
			scl_sync_dl <= '0';
		elsif (rising_edge(clk10x)) then
			sda_sync <= to_x01(sda);
			scl_sync <= to_x01(scl);
			sda_sync_dl <= sda_sync;
			scl_sync_dl <= scl_sync;
		end if;
	end process;

	--get a clock: use 50MHz to divide by the divider
	clk_div_10x : clk_div
		generic map(MAXD => divider)
		port map(clk => clk, reset => reset, div => divider, div_clk => clk10x);

		main_slave_sm : process (clk10x, reset)
		begin
			if reset = '1' then
				slv_state <= READY;
				rx_cnt <= (others => '0');
				tx_cnt <= to_unsigned(7, 3);
				data_reg <= (others => '0');
				addr_reg <= (others => '0');
				wr_reg <= (others => '0');
				rd_reg <= (others => '0');
				sda_i <= '0';
				datacnt <= '0';
			elsif rising_edge(clk10x) then
				slv_state <= slv_state_next;
				rx_cnt <= rx_cnt_next;
				tx_cnt <= tx_cnt_next;
				data_reg <= data_reg_next;
				addr_reg <= addr_reg_next;
				wr_reg <= wr_reg_next;
				rd_reg <= rd_reg_next;
				sda_i <= sda_i_next;
				datacnt <= datacnt_next;
			end if;
		end process;
		--next state logics in a two-segmented approach
		main_slave_sm_next : process (clk10x, reset, sda_i, tx_cnt, slv_state, sda_sync, scl_sync, sda_sync_dl, datacnt, scl_sync_dl, rx_cnt, data_reg, wr_reg, rd_reg, regin, addr_reg)
		begin
			case slv_state is
				when READY => 
					data_reg_next <= (others => '0'); --reset addr value
					rx_cnt_next <= (others => '0');
					sda_i_next <= '1'; --sitting high if not used
					wr_reg_next <= wr_reg;
					rd_reg_next <= rd_reg;
					addr_reg_next <= addr_reg;
					if (sda_sync_dl = '1' and sda_sync = '0') and (scl_sync_dl = '1' and scl_sync = '1') then --detects a downward transition on the sda line while no change on scl line
						slv_state_next <= DEVICEID;
					else
						slv_state_next <= READY;
					end if;
					datacnt_next <= '0';
					tx_cnt_next <= to_unsigned(7, 3);
				when DEVICEID => --this state gets the device id, if it matches with the id then send ack signal. Otherwise do nothing.
					wr_reg_next <= wr_reg;
					rd_reg_next <= rd_reg;
					addr_reg_next <= addr_reg;
					datacnt_next <= datacnt;
					tx_cnt_next <= to_unsigned(7, 3);
					if rx_cnt < 8 then
						if (scl_sync_dl = '0' and scl_sync = '1') then --detects a rising edge of scl
							--latch data
							data_reg_next <= data_reg(6 downto 0) & sda_sync;
							rx_cnt_next <= rx_cnt + 1;
						else
							data_reg_next <= data_reg; --keep value
							rx_cnt_next <= rx_cnt;
						end if;
						slv_state_next <= DEVICEID; --
						sda_i_next <= '1';
					else
						--wait for the falling edge before making an action ...
						if (scl_sync_dl = '1' and scl_sync = '0') then
							if data_reg(7 downto 1) = ID then --(7 downto 1) is the id, 0th bit is the R/W bit
								slv_state_next <= SLV_ACK1;
								sda_i_next <= '1';
								rx_cnt_next <= (others => '0');
							else
								slv_state_next <= READY;
								sda_i_next <= '1';
								rx_cnt_next <= rx_cnt;
							end if;
						else
							slv_state_next <= DEVICEID;
							sda_i_next <= '1';
							rx_cnt_next <= rx_cnt;
						end if;
						data_reg_next <= data_reg;
					end if;
				when SLV_ACK1 => 
					wr_reg_next <= wr_reg;
					rd_reg_next <= rd_reg;
					addr_reg_next <= addr_reg;
					datacnt_next <= datacnt;
					--wait for clock falling edge ...
					if (scl_sync_dl = '1' and scl_sync = '0') then
						if data_reg(0) = '0' then
							slv_state_next <= WRVALUE;
							data_reg_next <= (others => '0'); --reset data value
							sda_i_next <= '0';
							tx_cnt_next <= tx_cnt;
						else
							slv_state_next <= RDVALUE;
							data_reg_next <= regin;
							sda_i_next <= regin(to_integer(tx_cnt));
							tx_cnt_next <= to_unsigned(7, 3);
						end if;
					else
						sda_i_next <= '0';
						slv_state_next <= SLV_ACK1;
						data_reg_next <= data_reg; --keep addr value
						tx_cnt_next <= tx_cnt;
					end if;
					rx_cnt_next <= rx_cnt;
				when WRVALUE => 
					rd_reg_next <= rd_reg;
					datacnt_next <= datacnt;
					tx_cnt_next <= to_unsigned(7, 3);
					if rx_cnt < 8 then
						if ((sda_sync_dl = '1' and sda_sync = '0') and (scl_sync_dl = '1' and scl_sync = '1')) then --this is a start bit!
							slv_state_next <= DEVICEID; --
							rx_cnt_next <= (others => '0');
							data_reg_next <= data_reg;
						elsif (scl_sync_dl = '0' and scl_sync = '1') then --detects a rising edge of scl
							--latch data
							data_reg_next <= data_reg(6 downto 0) & sda_sync;
							rx_cnt_next <= rx_cnt + 1;
							slv_state_next <= WRVALUE; --
						else
							data_reg_next <= data_reg; --keep value
							rx_cnt_next <= rx_cnt;
							slv_state_next <= WRVALUE; --
						end if;
						sda_i_next <= '1';
						wr_reg_next <= wr_reg;
						addr_reg_next <= addr_reg;
					else
						if (scl_sync_dl = '1' and scl_sync = '0') then
							slv_state_next <= SLV_ACK2;
							data_reg_next <= data_reg;
							rx_cnt_next <= (others => '0');
							sda_i_next <= '0';
							if datacnt = '0' then
								wr_reg_next <= wr_reg;
								addr_reg_next <= data_reg;
							else
								wr_reg_next <= data_reg;
								addr_reg_next <= addr_reg;
							end if;
						else
							data_reg_next <= data_reg; --keep value
							rx_cnt_next <= rx_cnt;
							slv_state_next <= WRVALUE;
							sda_i_next <= '1';
							wr_reg_next <= wr_reg;
							addr_reg_next <= addr_reg;
						end if;
					end if;
				when SLV_ACK2 => 
					sda_i_next <= '0';
					wr_reg_next <= wr_reg;
					rd_reg_next <= rd_reg;
					addr_reg_next <= addr_reg;
					tx_cnt_next <= to_unsigned(7, 3);
					data_reg_next <= data_reg; --keep addr value
					--wait for the falling edge
					if (scl_sync_dl = '1' and scl_sync = '0') then
						if datacnt = '0' then
							slv_state_next <= WRVALUE;
							datacnt_next <= '1';
						else
							slv_state_next <= STOP;
							datacnt_next <= datacnt;
						end if;
						rx_cnt_next <= (others => '0');
					else
						slv_state_next <= SLV_ACK2;
						rx_cnt_next <= rx_cnt + 1;
						datacnt_next <= datacnt;
					end if;
				when RDVALUE => 
					wr_reg_next <= wr_reg;
					rd_reg_next <= rd_reg;
					datacnt_next <= datacnt;
					addr_reg_next <= addr_reg;
					data_reg_next <= data_reg; 
					if (scl_sync_dl = '1' and scl_sync = '0') then
						if tx_cnt = 0 then
							tx_cnt_next <= tx_cnt;
							slv_state_next <= PRESTOP;
						else
							tx_cnt_next <= tx_cnt - 1;
							slv_state_next <= RDVALUE;
						end if;
					else
						tx_cnt_next <= tx_cnt;
						slv_state_next <= RDVALUE;
					end if;
					sda_i_next <= regin(to_integer(tx_cnt));
					rx_cnt_next <= rx_cnt;
				when PRESTOP => 
					--wait for a falling edge
					if (scl_sync_dl = '1' and scl_sync = '0') then
						slv_state_next <= PRESTOP;
						sda_i_next <= sda_i;
						rx_cnt_next <= rx_cnt;
					else
						slv_state_next <= STOP;
						sda_i_next <= '1';
						rx_cnt_next <= (others => '0');
					end if;
					tx_cnt_next <= tx_cnt;
					wr_reg_next <= wr_reg;
					rd_reg_next <= rd_reg;
					datacnt_next <= datacnt;
					addr_reg_next <= addr_reg;
					data_reg_next <= data_reg;
				when STOP => 
					if (sda_sync_dl = '0' and sda_sync = '1' and scl_sync = '1' and scl_sync_dl = '1') then --detect the stop condition
						slv_state_next <= READY;
						rx_cnt_next <= rx_cnt;
					else
						if rx_cnt < 9 then --put a timeout and make it go back to READY state
							slv_state_next <= STOP;
							rx_cnt_next <= rx_cnt + 1;
						else
							slv_state_next <= READY;
							rx_cnt_next <= rx_cnt;
						end if;
					end if;
					data_reg_next <= data_reg;
					sda_i_next <= '1';
					wr_reg_next <= wr_reg;
					addr_reg_next <= addr_reg;
					datacnt_next <= datacnt;
					tx_cnt_next <= to_unsigned(7, 3);
			end case;
		end process;

		--set scl and sda outputs
		scl <= 'Z';
		sda <= '0' when sda_i = '0' else 'Z';

end Behavioral;