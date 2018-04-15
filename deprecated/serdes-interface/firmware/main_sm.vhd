----------------------------------------------------------------------------------
--This is the main state machine of the serdes FPGA
--it generates the appropriate command 
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all; 

entity main_sm is
port(
	clk_spi  : in std_logic; 
	reset    : in std_logic; 
	miso_reg : in std_logic_vector(15 downto 0); 
	data_lclkin : in std_logic; --this the signal that signal's end of a SPI command. 
	
	spi_start_o : out std_logic; 
	command_o : out std_logic_vector(15 downto 0);
	hsync_o : out std_logic
);
end main_sm;

architecture Behavioral of main_sm is

--state machine
type master_sm_type is (IDLE, REGCONF, ADCCONF, ACQ); 
signal master_sm, master_sm_next : master_sm_type;

type hsync_sm_type is (IDLE, CH0); 
signal hsync_state, hsync_state_next : hsync_sm_type;

--signals 
signal sm_cnt, sm_cnt_next : unsigned(5 downto 0); 
signal cmd, cmd_next : std_logic_vector(15 downto 0); 
signal cmd_d1, cmd_d2 : std_logic_vector(7 downto 0); --this is the delay version of command. currently only use for checking the configurations
signal spi_start, spi_start_next : std_logic; 
signal verify_cnt, verify_cnt_next : unsigned(5 downto 0); 
signal hsync_cnt, hsync_cnt_next : unsigned(4 downto 0); 
signal hsync, hsync_next : std_logic; 

--a bank of all the configuration values 
type rom_type is array ( 0 to 21) of std_logic_vector(7 downto 0);
type dummyrom_type is array (0 to 3) of std_logic_vector(15 downto 0); 
constant CONVERT:   std_logic_vector(1 downto 0) := "00";
constant CALIB: std_logic_vector(15 downto 0) := "0101010100000000";
constant CLEAR: std_logic_vector(15 downto 0) := "0110101000000000";
constant WRITEREG: std_logic_vector(1 downto 0) := "10";
constant READREG: std_logic_vector(1 downto 0) := "11";
constant NO_CONF_REG : integer := 21; --17 for 32 channel 
constant DUMMY_ROM : dummyrom_type := (
	"11" & std_logic_vector(to_unsigned(40,6)) & "00000000",
	"11" & std_logic_vector(to_unsigned(41,6)) & "00000000",
	"11" & std_logic_vector(to_unsigned(42,6)) & "00000000",
	"11" & std_logic_vector(to_unsigned(43,6)) & "00000000"
);

--generate command 
--command <= "11" & std_logic_vector(to_unsigned(41,6)) & "00000000"; --read from 40 to 44 registers
--configuration sequence
--				  7654 3210 
--R0  0x80DE "1101 1110" 
--R1  0x8102 "0000 0010" -ADC buffer bias, 2 for >700 KS/s sampling rate. 
--R2  0x8204 "0000 0100" -MUX bias 4 for >700 KS/s sampling rate 
--R3  0x8302 "0000 0010" -digital out HiZ
--R4  0x845F "0101 1111" -MISO pull to highZ when CS is pulled high. twocomp. no absmode, DSP offset remove, k_freq = 0.000004857Hz
--R5  0x8500 "0000 0000" -disable impedance check 
--R6  0x8600 "0000 0000" -disable impedance check DAC
--R7  0x8700 "0000 0000" -disable impedance check amplifier
--R8  0x8811 "0001 0001" -RH1 DAC1: 17 upper cutoff 10KHz
--R9  0x8980 "1000 0000" -RH1 DAC2: 0  
--R10 0x8A10 "0001 0000" -RH2 DAC1: 16 
--R11 0x8B80 "1000 0000" -RH2 DAC2: 0
--R12 0x8C10 "0001 0000" -RL  DAC1
--R13 0x8DDC "1101 1100" -RL DAC2:28 DAC3:1 cutoff: 0.1HZ??????????????????????? confirm
--R14 0x8EFF "1111 1111" 
--R15 0x8FFF "1111 1111"
--R16 0x90FF "1111 1111"
--R17 0x91FF "1111 1111"
--for 64 channels 
--R18 0x8EFF "1111 1111" 
--R19 0x8FFF "1111 1111"
--R20 0x90FF "1111 1111"
--R21 0x91FF "1111 1111"

constant CONFIG_ROM : rom_type := (
	--	 76543210
		"11011110", --0x80DE
		"00000010", --0x8102
		"00000100", --0x8204
		"00000010", --0x8302
		"00011111", --0x845F
		"00000000", --0x8500
		"00000000", --0x8600
		"00000000", --0x8700
		"00010001", --0x8811
		"10000000", --0x8980
		"00010000", --0x8A10
		"10000000", --0x8B80
		"00010000", --0x8C10
		"11011100", --0x8DDC
		"11111111", --0x8EFF
		"11111111", --0x8FFF
		"11111111", --0x90FF
		"11111111",
		"11111111", --0x8EFF
		"11111111", --0x8FFF
		"11111111", --0x90FF
		"11111111"); --0x91FF
		
begin

--signal mapping 
command_o <= cmd; 
spi_start_o <= spi_start;
hsync_o <= hsync; 

--delay the cmd output with data_lclk
delay_cmd_prc : process(data_lclkin, clk_spi, reset, cmd_d1) 
begin 
	if (reset = '1') then 
		cmd_d1 <= (others=>'0'); 
		cmd_d2 <= (others=>'0'); 
	elsif (rising_edge(clk_spi)) then 
		if data_lclkin = '1' then 
			cmd_d1 <= cmd(7 downto 0); 
			cmd_d2 <= cmd_d1;
		else 
			cmd_d1 <= cmd_d1; 
			cmd_d2 <= cmd_d2;
		end if; 
	end if; 
end process; 

--Main state machine 
main_proc: process(clk_spi, reset) 
begin 
	if (reset = '1') then 
		master_sm <= IDLE; 
		sm_cnt <= (others=>'0');
		cmd <= (others=>'0'); 
		verify_cnt <= (others=>'0'); 
		spi_start <= '0'; 
	elsif (rising_edge(clk_spi)) then --next state logic
		master_sm <= master_sm_next;
		sm_cnt <= sm_cnt_next; 
		cmd <= cmd_next; 
		verify_cnt <= verify_cnt_next; 
		spi_start <= spi_start_next; 
	end if; 
end process;

--next state logic
main_proc_next: process(data_lclkin, sm_cnt, master_sm, cmd, cmd_d2, miso_reg, verify_cnt) 
begin 
	case master_sm is 
		when IDLE => 
			master_sm_next <= REGCONF; 
			spi_start_next <= '1';
			sm_cnt_next <= sm_cnt + 1; 
			cmd_next <= WRITEREG & std_logic_vector(sm_cnt) & CONFIG_ROM(to_integer(sm_cnt));
			verify_cnt_next <= (others=>'0');
		when REGCONF => --go through all the configuration registers (generate command, and spi_start signal, look for data_lclkin before moving to the next state) 
			if data_lclkin = '1' then 
				if sm_cnt <= 2 then 
					sm_cnt_next <= sm_cnt + 1; 
					master_sm_next <= REGCONF; 
					cmd_next <= WRITEREG & std_logic_vector(sm_cnt) & CONFIG_ROM(to_integer(sm_cnt));
					verify_cnt_next <= verify_cnt; 
					spi_start_next <= '1';
				elsif sm_cnt <= NO_CONF_REG and sm_cnt > 2 then 
					sm_cnt_next <= sm_cnt + 1; 
					master_sm_next <= REGCONF; 
					cmd_next <= WRITEREG & std_logic_vector(sm_cnt) & CONFIG_ROM(to_integer(sm_cnt));
					if miso_reg(7 downto 0) = cmd_d2(7 downto 0) then 
						verify_cnt_next <= verify_cnt + 1; 
					else 
						verify_cnt_next <= verify_cnt; 
					end if; 
					spi_start_next <= '1';
				elsif sm_cnt > NO_CONF_REG and sm_cnt <= (NO_CONF_REG + 3) then --this is the last of the verification period 
					sm_cnt_next <= sm_cnt + 1; 
					master_sm_next <= REGCONF; 
					cmd_next <= (others=>'0');
					if miso_reg(7 downto 0) = cmd_d2(7 downto 0) then 
						verify_cnt_next <= verify_cnt + 1; 
					else 
						verify_cnt_next <= verify_cnt; 
					end if; 
					spi_start_next <= '1';
				else 			--when sm_cnt > 20		
					if verify_cnt = 22 then 
						master_sm_next <= ADCCONF;
						sm_cnt_next <= (others=>'0');
						spi_start_next <= '1';
						cmd_next <= CALIB; --initiate the calibration command 
					else  --otherwise stuck in REGCONF
						master_sm_next <= REGCONF; --debug change
						sm_cnt_next <= sm_cnt;
						spi_start_next <= '0';
						cmd_next <= (others=>'0'); 
					end if; 
					verify_cnt_next <= verify_cnt; 
				end if; 
			else 
				sm_cnt_next <= sm_cnt;
				spi_start_next <= '0'; 
				master_sm_next <= master_sm;
				cmd_next <= cmd; 
				verify_cnt_next <= verify_cnt;
			end if; 		
		when ADCCONF => 
			if data_lclkin = '1' then 
				if sm_cnt <= 50 then --9
					sm_cnt_next <= sm_cnt + 1; 
					master_sm_next <= ADCCONF; 
					cmd_next <= DUMMY_ROM(0);
				else 
					sm_cnt_next <= (others=>'0');
					master_sm_next <= ACQ;
					cmd_next <= cmd;
				end if; 
				spi_start_next <= '1';
			else 
				sm_cnt_next <= sm_cnt;
				spi_start_next <= '0'; --debug 
				master_sm_next <= master_sm;
				cmd_next <= cmd; 
			end if; 
			verify_cnt_next <= verify_cnt;
		when ACQ => 
			if data_lclkin = '1' then 
				if sm_cnt >= 34 then --reset channel count back to 0 
					sm_cnt_next <= (others=>'0'); 
				else 
					sm_cnt_next <= sm_cnt + 1; 
				end if; 
				cmd_next <= "00" & std_logic_vector(sm_cnt) & "00000000";
				--cmd_next <= "11" & std_logic_vector(to_unsigned(59,6)) & "00000000"; --read from 40 to 44 registers	--read for INTAN 
				spi_start_next <= '1';
			else 
				sm_cnt_next <= sm_cnt;
				spi_start_next <= '0'; 
				cmd_next <= cmd; 
			end if; 
			master_sm_next <= ACQ;
			verify_cnt_next <= verify_cnt;
	end case; 
end process; 

--one shot hsync for channel 0 
one_shot_hsync : process(clk_spi, reset) 
begin 
	if (reset = '1') then 
		hsync_state <= IDLE; 
		hsync_cnt <= (others=>'0'); 
		hsync <= '0'; 
	elsif (rising_edge(clk_spi)) then 
		hsync_state <= hsync_state_next; 
		hsync_cnt <= hsync_cnt_next; 
		hsync <= hsync_next; 
	end if; 
end process; 

--hsync
one_shot_next_proc : process(master_sm, hsync_state, sm_cnt, hsync_cnt, data_lclkin)
begin 
	case hsync_state is 
		when IDLE => 
			if master_sm = ACQ and sm_cnt = 1 and data_lclkin = '1' then 
				--go to the CH0 state
				hsync_state_next <= CH0; 
				hsync_next <= '1'; 
			else 
				hsync_state_next <= IDLE ; 
				hsync_next <= '0'; 
			end if; 
			hsync_cnt_next <= (others=>'0'); 
		when CH0 => 
				if hsync_cnt >= 10 then 
					hsync_state_next <= IDLE;
					hsync_cnt_next <= (others=>'0'); 
					hsync_next <= '0'; 
				else 
					hsync_state_next <= CH0;
					hsync_cnt_next <= hsync_cnt + 1;
					hsync_next <= '1'; 
				end if; 
	end case; 
end process; 

end Behavioral;

