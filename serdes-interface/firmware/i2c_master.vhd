----------------------------------------------------------------------------------
--This is adapted from this code: https://eewiki.net/pages/viewpage.action?pageId=10125324
--modification: now sends 3 I2C packets with only a single ena signal pulse:
--write: [device id | register address | value]
--read: [device id |w| register address |device id|r| value]
--Also had to modify it to do the correct read sequence
--fixed the scl stretch bug
--by: Jie (Jack) Zhang MWL-MIT
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

entity i2c_master is
	generic (
		input_clk : integer := 20_000_000; --input clock speed from user logic in Hz
	bus_clk : integer := 200_000); --speed the i2c bus (scl) will run at in Hz
	port (
		clk             : in std_logic; --system clock
		reset           : in std_logic; --active high reset
		ena             : in std_logic; --latch in command
		devid           : in std_logic_vector(6 downto 0); --device id of target slave
		addr            : in std_logic_vector(7 downto 0);
		rw              : in std_logic; --'0' is write, '1' is read
		data_wr         : in std_logic_vector(7 downto 0); --data to write to slave
		busy            : out std_logic; --indicates transaction in progress
		data_rd         : out std_logic_vector(7 downto 0); --data read from slave
		ack_error       : buffer std_logic; --flag if improper acknowledge from slave
		rd_from_remote  : in std_logic;
		sda             : inout std_logic; --serial data output of i2c bus
	scl : inout std_logic); --serial clock output of i2c bus
end i2c_master;

architecture Behavioral of i2c_master is

	constant divider : integer := (input_clk/bus_clk)/4; --number of clocks in 1/4 cycle of scl
	type machine is(ready, start, command, slv_ack1, wr, rd, slv_ack2, mstr_ack, stop); --needed states
	signal state : machine; --state machine
	signal data_clk : std_logic; --data clock for sda
	signal data_clk_prev : std_logic; --data clock during previous system clock
	signal scl_clk : std_logic; --constantly running internal scl
	signal scl_ena : std_logic := '0'; --enables internal scl to output
	signal sda_int : std_logic := '1'; --internal sda
	signal sda_ena_n : std_logic; --enables internal sda to output
	signal devid_rw : std_logic_vector(7 downto 0); --latched in device id and read/write
	signal regaddr : std_logic_vector(7 downto 0);
	signal data_tx : std_logic_vector(7 downto 0); --latched in data to write to slave
	signal data_rx : std_logic_vector(7 downto 0); --data received from slave
	signal bit_cnt : integer range 0 to 7 := 7; --tracks bit number in transaction
	signal stretch : std_logic := '0'; --identifies if slave is stretching scl
	signal datacnt : std_logic := '0';
	signal rd_cnt : std_logic := '0';
	signal scl_cnt : unsigned(6 downto 0);
begin
	--generate the timing for the bus clock (scl_clk) and the data clock (data_clk)
	process (clk, reset, scl)
	variable count : integer range 0 to divider * 4; --timing for clock generation
	begin
		--count_unsigned <= to_unsigned(count, 11);
		if (reset = '1') then --reset asserted
			stretch <= '0';
			count := 0;
		elsif (rising_edge(clk)) then
			data_clk_prev <= data_clk; --store previous value of data clock
			if (count = divider * 4 - 1) then --end of timing cycle
				count := 0; --reset timer
			elsif (stretch = '0') then --clock stretching from slave not detected
				count := count + 1; --continue clock generation timing
			end if;
			case count is
				when 0 to divider - 1 => --first 1/4 cycle of clocking
					scl_clk <= '0';
					data_clk <= '0';
				when divider to divider * 2 - 1 => --second 1/4 cycle of clocking
					scl_clk <= '0';
					data_clk <= '1';
				when divider * 2 to divider * 3 - 1 => --third 1/4 cycle of clocking
					scl_clk <= '1'; --release scl
					if count = (divider * 2 + 1) then
						if (scl = '0') then --detect if slave is stretching clock
							stretch <= '1';
						else
							stretch <= '0';
						end if;
					end if;
					data_clk <= '1';
				when others => --last 1/4 cycle of clocking
					scl_clk <= '1';
					data_clk <= '0';
			end case;
		end if;
 
		scl_cnt <= to_unsigned(count, 7);
	end process;

	--state machine and writing to sda during scl low (data_clk rising edge)
	process (clk, reset, datacnt, rd_cnt, bit_cnt, rd_from_remote)
		begin
			if (reset = '1') then --reset asserted
				state <= ready; --return to initial state
				busy <= '1'; --indicate not available
				scl_ena <= '0'; --sets scl high impedance
				sda_int <= '1'; --sets sda high impedance
				ack_error <= '0'; --clear acknowledge error flag
				bit_cnt <= 7; --restarts data bit counter
				data_rd <= "00000000"; --clear data read port
				datacnt <= '0';
				rd_cnt <= '0';
			elsif (rising_edge(clk)) then
				if (data_clk = '1' and data_clk_prev = '0') then --data clock rising edge
					case state is
						when ready => --idle state
							if (ena = '1') then --transaction requested
								busy <= '1'; --flag busy
								devid_rw <= devid & rw; --collect requested slave device id and command
								data_tx <= data_wr; --collect requested data to write
								regaddr <= addr;
								state <= start; --go to start bit
							else --remain idle
								busy <= '0'; --unflag busy
								state <= ready; --remain idle
							end if;
							datacnt <= '0'; --reset data counter
							rd_cnt <= '0'; --set the ready counter
						when start => --start bit of transaction
							busy <= '1'; --resume busy if continuous mode
							sda_int <= devid_rw(bit_cnt); --set first device id bit to bus
							state <= command; --go to command
						when command => --device id and command byte of transaction
							if rd_from_remote = '0' then
								if (bit_cnt = 0) then --command transmit finished
									sda_int <= '1'; --release sda for slave acknowledge
									bit_cnt <= 7; --reset bit counter for "byte" states
									state <= slv_ack1; --go to slave acknowledge (command)
								elsif (bit_cnt = 1) then
									if devid_rw(0) = '0' then
										sda_int <= devid_rw(bit_cnt - 1);
									else
										if rd_cnt = '0' then
											sda_int <= '0';
										else 
											sda_int <= '1';
										end if;
									end if;
									bit_cnt <= bit_cnt - 1;
									state <= command;
								else --next clock cycle of command state
									bit_cnt <= bit_cnt - 1; --keep track of transaction bits
									state <= command; --continue with command
									sda_int <= devid_rw(bit_cnt - 1); --write device id/command bit to bus
								end if;
							else
								if (bit_cnt = 0) then --command transmit finished
									sda_int <= '1'; --release sda for slave acknowledge
									bit_cnt <= 7; --reset bit counter for "byte" states
									state <= slv_ack1; --go to slave acknowledge (command)
								elsif (bit_cnt = 1) then
									if devid_rw(0) = '0' then
										sda_int <= devid_rw(bit_cnt - 1);
										state <= command;
										bit_cnt <= bit_cnt - 1;
									else
										if rd_cnt = '0' then
											sda_int <= '1';
											bit_cnt <= bit_cnt - 1;
											state <= command;
										else 
											sda_int <= '1'; --release sda for slave acknowledge
											bit_cnt <= 7; --reset bit counter for "byte" states
											state <= slv_ack1; --go to slave acknowledge (command)
										end if;
									end if;
								else --next clock cycle of command state
									bit_cnt <= bit_cnt - 1; --keep track of transaction bits
									state <= command; --continue with command
									sda_int <= devid_rw(bit_cnt - 1); --write device id/command bit to bus
								end if;
							end if;
						when slv_ack1 => --slave acknowledge bit (command)
							if (devid_rw(0) = '0') then --write command
								sda_int <= regaddr(bit_cnt); --write first bit of data
								state <= wr; --go to write byte
							else --read command
								if rd_cnt = '0' then
									sda_int <= regaddr(bit_cnt); --write first bit of data
									state <= wr; --go to write byte
									rd_cnt <= '1';
								else
									sda_int <= '1'; --release sda from incoming data
									state <= rd; --go to read byte
								end if;
							end if;
						when wr => --write byte of transaction
							busy <= '1'; --resume busy if continuous mode
							if (bit_cnt = 0) then --write byte transmit finished
								sda_int <= '1'; --release sda for slave acknowledge
								bit_cnt <= 7; --reset bit counter for "byte" states
								state <= slv_ack2; --go to slave acknowledge (write)
							else --next clock cycle of write state
								bit_cnt <= bit_cnt - 1; --keep track of transaction bits
								if datacnt = '0' then
									sda_int <= regaddr(bit_cnt - 1); --write next bit to bus
								else
									sda_int <= data_tx(bit_cnt - 1); --write next bit to bus
								end if;
								state <= wr; --continue writing
							end if;
						when rd => --read byte of transaction
							busy <= '1'; --resume busy if continuous mode
							if (bit_cnt = 0) then --read byte receive finished
								if (ena = '1' and devid_rw = devid & rw) then --continuing with another read at same device id
									sda_int <= '0'; --acknowledge the byte has been received
								else --stopping or continuing with a write
									sda_int <= '1'; --send a no-acknowledge (before stop or repeated start)
								end if;
								bit_cnt <= 7; --reset bit counter for "byte" states
								data_rd <= data_rx; --output received data
								state <= mstr_ack; --go to master acknowledge
							else --next clock cycle of read state
								bit_cnt <= bit_cnt - 1; --keep track of transaction bits
								state <= rd; --continue reading
							end if;
						when slv_ack2 => --slave acknowledge bit (write)
							if rw = '0' then --write
								if (datacnt = '0') then --continue transaction
									busy <= '1'; --continue is accepted
									--devid_rw <= devid & rw; --collect requested slave device id and command
									sda_int <= data_tx(bit_cnt); --write first bit of data
									state <= wr; --go to write byte
									datacnt <= '1';
								else --complete transaction
									state <= stop; --go to stop bit
								end if;
							else
								state <= start;
								devid_rw <= devid & rw;
								bit_cnt <= 7;
							end if;
						when mstr_ack => --master acknowledge bit after a read
							if (ena = '1') then --continue transaction
								busy <= '0'; --continue is accepted and data received is available on bus
								devid_rw <= devid & rw; --collect requested slave device id and command
								data_tx <= data_wr; --collect requested data to write
								if (devid_rw = devid & rw) then --continue transaction with another read
									sda_int <= '1'; --release sda from incoming data
									state <= rd; --go to read byte
								else --continue transaction with a write or new slave
									state <= start; --repeated start
								end if;
							else --complete transaction
								state <= stop; --go to stop bit
							end if;
						when stop => --stop bit of transaction
							busy <= '0'; --unflag busy
							state <= ready; --go to idle state
					end case;
				elsif (data_clk = '0' and data_clk_prev = '1') then --data clock falling edge
					case state is
						when start => 
							if (scl_ena = '0') then --starting new transaction
								scl_ena <= '1'; --enable scl output
								ack_error <= '0'; --reset acknowledge error output
							end if;
						when slv_ack1 => --receiving slave acknowledge (command)
							if (sda /= '0' or ack_error = '1') then --no-acknowledge or previous no-acknowledge
								ack_error <= '1'; --set error output if no-acknowledge
							end if;
						when rd => --receiving slave data
							data_rx(bit_cnt) <= sda; --receive current slave data bit
						when slv_ack2 => --receiving slave acknowledge (write)
							if (sda /= '0' or ack_error = '1') then --no-acknowledge or previous no-acknowledge
								ack_error <= '1'; --set error output if no-acknowledge
							end if;
						when stop => 
							scl_ena <= '0'; --disable scl
						when others => 
							null;
					end case;
				end if;
			end if;
		end process;

		--set sda output
		with state select
		sda_ena_n <= data_clk_prev when start, --generate start condition
		             not data_clk_prev when stop, --generate stop condition
		             sda_int when others; --set to internal sda signal

		--set scl and sda outputs
		scl <= '0' when (scl_ena = '1' and scl_clk = '0') else 'Z';
		sda <= '0' when sda_ena_n = '0' else 'Z';

end Behavioral;