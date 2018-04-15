----------------------------------------------------------------------------------
--This is a SPI module that takes a parallel command and stream to SPI outputs
----------------------------------------------------------------------------------
library IEEE;
use ieee.numeric_std.all;
use IEEE.STD_LOGIC_1164.ALL;

entity SPI_module is
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
	--data clock 
	data_rdy_pcie_o : out std_logic;
	data_pcie_A_o : out std_logic_vector(15 downto 0); 
	data_pcie_B_o : out std_logic_vector(15 downto 0); 
	miso_reg_A_o : out std_logic_vector(15 downto 0);
	miso_reg_B_o : out std_logic_vector(15 downto 0) 
);
end SPI_module;

architecture Behavioral of SPI_module is

signal cs, cs_next, sclk, sclk_next : std_logic;
signal miso_reg_A, miso_reg_A_next : std_logic_vector(15 downto 0);
signal miso_reg_B, miso_reg_B_next : std_logic_vector(15 downto 0);
signal sm_cnt, sm_cnt_next : unsigned(4 downto 0); --max count is (41-34) = 12 
signal cmd_reg, cmd_reg_next : std_logic_vector(15 downto 0); 
type spi_states is (IDLE, IDLExN, OP_lo, OP_hi, DATARDY); --state machine definition
signal spi_sm, spi_sm_next : spi_states;  
signal data_lclk, data_lclk_next : std_logic; 
signal data_rdy_pcie, data_rdy_pcie_next : std_logic; 
signal data_pcie_A, data_pcie_A_next : std_logic_vector(15 downto 0); 
signal data_pcie_B, data_pcie_B_next : std_logic_vector(15 downto 0); 

constant test_miso: std_logic_vector(0 to 15) := "0100000000000000";
begin

--signal mapping
cs_o <= cs; 
sclk_o <= sclk; 
mosi_o <= cmd_reg(15); --comd_reg is a shift register so bit 15 goes to MOSI output
data_lclk_o <= data_lclk; 
data_rdy_pcie_o <= data_rdy_pcie;
data_pcie_A_o <= data_pcie_A;
data_pcie_B_o <= data_pcie_B;
miso_reg_A_o <= miso_reg_A;
miso_reg_B_o <= miso_reg_B;

--SPI state machine 
SPI_proc: process(clk_spi, reset) 
begin 
	if (reset = '1') then 
		cs <= '0'; 
		sclk <= '0';
		data_lclk <= '0'; 
		miso_reg_A <= (others=>'0');
		miso_reg_B <= (others=>'0'); 
		spi_sm <= IDLE; 
		cmd_reg <= (others=>'0'); 
		sm_cnt <= (others=>'0');  --sm counter 
		data_rdy_pcie <= '0'; 
		data_pcie_A <= (others=>'0'); 
		data_pcie_B <= (others=>'0'); 
	elsif (falling_edge(clk_spi)) then --next state logic
		cs <= cs_next; 
		sclk <= sclk_next; 
		data_lclk <= data_lclk_next;
		miso_reg_A <= miso_reg_A_next;
		miso_reg_B <= miso_reg_B_next; 		
		spi_sm <= spi_sm_next; 
		cmd_reg <= cmd_reg_next;
		sm_cnt <= sm_cnt_next;
		data_rdy_pcie <= data_rdy_pcie_next; 
		data_pcie_A <= data_pcie_A_next; 
		data_pcie_B <= data_pcie_B_next; 
	end if; 
end process; 

--state machine next state===============================
--on the SPI output side: 
	--it toggles between OP_lo and OP_hi state until sm_cnt reaches 16 counts then it goes to the DATARDY state. 
	--The DATARDY state toggles the data clock high, which means the data is ready to be latched. Then it goes back to 
	--IDLE state to wait for next spi_start signal
--on the SPI input side: 
	--MISO shifts the incoming bits into a miso shift register 
	



SPI_proc_next: process(SPI_sm, sclk, sm_cnt, cmd_reg, miso_reg_b, data_pcie_b, spi_start, miso_reg_A, command_in, miso_i, data_pcie_A) 
begin 
	case SPI_sm is 
		when IDLE => 
			if spi_start = '1' then 
				cmd_reg_next <= command_in;  
				spi_sm_next <= IDLExN;
				data_rdy_pcie_next <= '1'; 				
			else 
				cmd_reg_next <= cmd_reg; 
				spi_sm_next <= IDLE; 
				data_rdy_pcie_next <= '0'; 
			end if; 
			cs_next <= '1'; 
			sclk_next <= '0';
			sm_cnt_next <= (others=>'0'); --state counter 
			data_lclk_next <= '0'; --data clock is always '0' unless it is in data_ready state; 
			miso_reg_A_next <= miso_reg_A; --maintain the last miso values, do not reset
			miso_reg_B_next <= miso_reg_B; 
			data_pcie_A_next <= data_pcie_A; 
			data_pcie_B_next <= miso_reg_B;
		when IDLExN => --loop in here for 24 cycles
			if sm_cnt >= 8 then 
				spi_sm_next <= OP_lo; 
				cs_next <= '0'; 
				sclk_next <= '0';
				sm_cnt_next <= (others=>'0'); 
				miso_reg_A_next <= (others=>'0'); --maintain the last miso values, do not reset
				miso_reg_B_next <= (others=>'0'); 	
			else 
				spi_sm_next <= IDLExN; 
				cs_next <= '1'; 
				sclk_next <= '0';
				sm_cnt_next <= sm_cnt + 1;
				miso_reg_A_next <= miso_reg_A; --maintain the last miso values, do not reset
				miso_reg_B_next <= miso_reg_B; 		
			end if; 
			cmd_reg_next <= cmd_reg; 
			data_lclk_next <= '0'; --data clock is always '0' unless it is in data_ready state; 
			data_rdy_pcie_next <= '1'; 
			data_pcie_A_next <= data_pcie_A;
			data_pcie_B_next <= data_pcie_B; 
		when OP_lo => 
			cmd_reg_next <= cmd_reg; 
			spi_sm_next <= OP_hi;
			sclk_next <= not sclk; --toggle sclk
			cs_next <= '0';
			sm_cnt_next <= sm_cnt;	
			data_lclk_next <= '0';		
			data_rdy_pcie_next <= '0'; 			
			miso_reg_A_next <= miso_reg_A;
			-------------------------------
			if sm_cnt = 0 then 
				miso_reg_B_next <= miso_reg_B;
			else
				miso_reg_B_next <= miso_reg_B(14 downto 0) & miso_i;
			end if; 
			-------------------------------
			data_pcie_A_next <= data_pcie_A;
			data_pcie_B_next <= data_pcie_B; 
		when OP_hi =>
			if sm_cnt>=15 then --state counter triggers at 15
				spi_sm_next <= DATARDY; 
				sm_cnt_next <= sm_cnt;
			else 
				spi_sm_next <= OP_lo; 
				sm_cnt_next <= sm_cnt + 1; --sm counter increment
			end if; 
			cmd_reg_next(15 downto 1) <= cmd_reg(14 downto 0); --shift the command out
			cmd_reg_next(0) <= '0'; --pad '0';
			sclk_next <= not sclk; --toggle sclk
			cs_next <= '0'; 
			data_lclk_next <= '0';
			data_rdy_pcie_next <= '0';
			miso_reg_A_next <= miso_reg_A(14 downto 0) & miso_i;
			data_pcie_A_next <= data_pcie_A;
			miso_reg_B_next <= miso_reg_B;
			data_pcie_B_next <= data_pcie_B; 
		when DATARDY => 
			spi_sm_next <= IDLE; 
			sm_cnt_next <= sm_cnt; 
			cmd_reg_next <= cmd_reg; 
			sclk_next <= '0'; 
			cs_next <= '1';
			data_lclk_next <= '1';
			data_rdy_pcie_next <= '0';
			miso_reg_A_next <= miso_reg_A;
			data_pcie_A_next <= miso_reg_A;
			--miso_reg_B_next <= miso_reg_B;
			miso_reg_B_next <= miso_reg_B(14 downto 0) & miso_i;
			data_pcie_B_next <= data_pcie_B; 
	end case; 
end process; 

end Behavioral;

