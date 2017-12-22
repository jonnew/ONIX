-- this is use to initilize the I2C pass through configuration need to communicate with remote I2C devices.
-- this runs on the host PC side.
-- by: Jie (Jack) Zhang MWL-MIT
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

entity i2c_master_init is
	port (
		clk                    : in std_logic; --same clock for the i2c interface 
		reset                  : in std_logic;
		busy                   : in std_logic;
		ack_error              : in std_logic;
		i2c_ena_o              : out std_logic;
		rw_o                   : out std_logic;
		device_id_o            : out std_logic_vector(6 downto 0);
		addr_o                 : out std_logic_vector(7 downto 0);
		value_o                : out std_logic_vector(7 downto 0);
		user_rw                : in std_logic;
		user_device_id         : in std_logic_vector(6 downto 0);
		user_addr              : in std_logic_vector(7 downto 0);
		user_value             : in std_logic_vector(7 downto 0)
	);
end i2c_master_init;

architecture Behavioral of i2c_master_init is

	--state machine
	type init_sm_type is (IDLE, CONF, TX, ACKERR, CONFBUSY, TXBUSY);
	signal init_sm, init_sm_next : init_sm_type;

	--signals
	signal addr, addr_next : std_logic_vector(7 downto 0);
	signal value, value_next : std_logic_vector(7 downto 0);
	signal device_id, device_id_next : std_logic_vector(6 downto 0);
	signal i2c_ena, i2c_ena_next : std_logic;
	signal rw, rw_next : std_logic;

	--counters
	signal confcnt, confcnt_next : unsigned(3 downto 0); --state counter
	constant CONF_SIZE : integer := 6;
	type addr_value_rom_type is array (0 to CONF_SIZE - 1) of std_logic_vector(7 downto 0);
	type deviceid_rom_type is array (0 to CONF_SIZE - 1) of std_logic_vector(6 downto 0);
	constant ADDR_ROM      : addr_value_rom_type := (
		"00100001", --0x21 (des)
		"00000111", --0x7 (des) (serializer alias)
		"00001000", --0x8 (des)
		"00010000", --0x10 (des)
		"00010001", --0x11 (ser)
		"00010010" --0x12 (ser)
	);
	constant VALUE_ROM     : addr_value_rom_type := (
		"00010111", --7: I2C passthrough (1) 6:4 I2C SDA hold (001) 3:0 I2C filter depth (0111)
		"10110000", --0x58<<1 ser alias
		"10100000", --0x50<<1 slave device ID
		"10100000", --0x50<<1 slave device alias
		"01100100", --0x64 for 100KHz SCL rate (high time)
		"01100100" --0x64 for 100KHz SCL rate (low time)
	);
	constant DEVICEID_ROM  : deviceid_rom_type := (
		"1100000", --"1100000": DES ID "1011000": SER ID
		"1100000", --des
		"1100000", --des
		"1100000", --des
		"1011000", --ser
		"1011000" --ser
	);

begin
	device_id_o <= device_id;
	addr_o <= addr; 
	value_o <= value;
	i2c_ena_o <= i2c_ena;
	rw_o <= rw;

	init_proc : process (clk, reset)
	begin
		if (reset = '1') then
			init_sm <= IDLE;
			confcnt <= (others => '0');
			addr <= (others => '0');
			value <= (others => '0');
			device_id <= (others => '0');
			i2c_ena <= '0';
			rw <= '0';
		elsif (rising_edge(clk)) then
			init_sm <= init_sm_next;
			confcnt <= confcnt_next;
			addr <= addr_next;
			value <= value_next;
			device_id <= device_id_next;
			i2c_ena <= i2c_ena_next;
			rw <= rw_next;
		end if;
	end process;

	init_proc_next : process (clk, reset, rw, user_rw, init_sm, confcnt, i2c_ena, busy, ack_error, value, addr, device_id, user_addr, user_value, user_device_id)
	begin
		case init_sm is
			when IDLE => 
				if busy = '0' then
					init_sm_next <= CONF;
				else
					init_sm_next <= IDLE;
				end if;
				i2c_ena_next <= '0';
				confcnt_next <= (others => '0');
				addr_next <= (others => '0');
				value_next <= (others => '0');
				device_id_next <= (others => '0');
				rw_next <= '0';
			when CONF => 
				i2c_ena_next <= '1'; --assert i2c enable signal
				rw_next <= '0'; --this is a write
				confcnt_next <= confcnt;
				if busy = '1' then
					init_sm_next <= CONFBUSY;
				else
					init_sm_next <= CONF;
				end if;
				addr_next <= ADDR_ROM(to_integer(confcnt));
				value_next <= VALUE_ROM(to_integer(confcnt));
				device_id_next <= DEVICEID_ROM(to_integer(confcnt));
			when CONFBUSY => 
				i2c_ena_next <= '0'; --disable enable pin
				rw_next <= rw; 
				if ack_error = '1' then
					init_sm_next <= ACKERR;
					confcnt_next <= confcnt; --do not increment this 
				elsif busy = '0' then --wait for busy go to low
					if (confcnt = CONF_SIZE - 1) then
						init_sm_next <= TX; --configuration is done
						confcnt_next <= confcnt;
					else
						init_sm_next <= CONF; --continue other configuration
						confcnt_next <= confcnt + 1; --increment the conf counter
					end if;
				else
					init_sm_next <= CONFBUSY;
					confcnt_next <= confcnt;
				end if;
				addr_next <= addr;
				value_next <= value;
				device_id_next <= device_id;
			when ACKERR => 
				if busy = '0' then
					init_sm_next <= CONF;
				else
					init_sm_next <= ACKERR;
				end if;
				confcnt_next <= confcnt;
				i2c_ena_next <= i2c_ena; 
				addr_next <= addr;
				value_next <= value;
				device_id_next <= device_id;
				rw_next <= rw; 
			when TX => 
				i2c_ena_next <= '1'; --assert i2c enable signal
				confcnt_next <= confcnt;
				if busy = '1' then
					init_sm_next <= TXBUSY;
				else
					init_sm_next <= TX;
				end if;
				rw_next <= user_rw;
				addr_next <= user_addr;
				value_next <= user_value;
				device_id_next <= user_device_id;
			when TXBUSY => 
				i2c_ena_next <= '0';
				if busy = '0' then --wait for busy to drop low
					init_sm_next <= TX;
				else
					init_sm_next <= TXBUSY;
				end if;
				rw_next <= rw;
				addr_next <= addr;
				value_next <= value;
				device_id_next <= device_id;
				confcnt_next <= confcnt;
		end case; 
	end process;

end Behavioral;