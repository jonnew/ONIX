--async_com_control.vdh
--by Jie Zhang, MWL, MIT.
--this module controls the async communication interface. It sends COBS encoded streams to the 8-bit width communication channel 
--it detects a magic word from the headstage, which symbolizes the transmission of configuration details of the headstage. 
--This module then encodes them using COBS before transmitting to the host. 

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;
library work;
use WORK.myDeclare.all;

entity async_com_control is
port (
	 bus_clk : in std_logic; 
    reset : in std_logic;
    --pclk  : in std_logic; 
    --din : in std_logic_vector(11 downto 0); --headstage communication input from the Deserilizer.
    dev_reset_in : in std_logic; --a signal that resets the state machine and gives out a device map data stream. 
    conf_ack : in std_logic; 
    conf_nack : in std_logic; 
    conf_done : in std_logic;
	 conf_mem_in : in mem_type; 
	 --cobs fifo output 
	 async_fifo_wr_enb : out std_logic; 
	 async_fifo_wr_data : out std_logic_vector(7 downto 0)
	 
    );
end async_com_control;

architecture Behavioral of async_com_control is

--COBS encoder declaration 
component cobs_encoder is
Port ( 
    bus_clk : in std_logic; 
    reset : in std_logic; 
	 --cobs inputs
    pre_cobs_data_in : in async_stream_type;
    data_in_length : in std_logic_vector(4 downto 0);
	 cobs_conv_begin : in std_logic;
	 --cobs outputs
    cobs_data_out : out cobs_stream_types;
    data_out_length : out std_logic_vector(4 downto 0);
	 cobs_conv_rdy : out std_logic
);
end component;

type async_sm_type is (IDLE, DEVRESET, COBSCONV, COBSWAIT, COBSPUSH); 
signal async_sm : async_sm_type; 


--CMD array struct
type async_cmd_array_type is array (0 to 8) of std_logic_vector(31 downto 0); 
--constant ASYNC_CMD_ARRAY : async_cmd_array_type := (
--x"00_00_00_00", --Configuration write ack 
--x"00_00_00_01", --Configuration write Nack 
--x"00_00_00_02", --Configuration read ack 
--x"00_00_00_04", --Configuration read Nack 
--x"00_00_00_08", --Configuration write ack 
--x"00_00_00_10", --DEVICE MAP START
--x"00_00_01_00", --FRAME READ SIZE IN BYTES
--x"00_00_10_00", --FRAME WRITE SIZE IN BYTES
--x"00_01_00_00"  --DEVICE MAP INSTANT
--);

constant ASYNC_CMD_ARRAY : async_cmd_array_type := (
"00000000000000000000000000000001", --Configuration write ack 
"00000000000000000000000000000010", --Configuration write Nack 
"00000000000000000000000000000100", --Configuration read ack 
"00000000000000000000000000001000", --Configuration read Nack 
"00000000000000000000000000010000", --Configuration write ack 
"00000000000000000000000000100000", --DEVICE MAP START
"00000000000000000000000001000000", --FRAME READ SIZE IN BYTES
"00000000000000000000000010000000", --FRAME WRITE SIZE IN BYTES
"00000000000000000000000100000000"  --DEVICE MAP INSTANT
);

constant DEVICEMAPACK : std_logic_vector(63 downto 0) := ASYNC_CMD_ARRAY(5) & std_logic_vector(to_unsigned(3,32)); 
constant FRAMERSIZE : std_logic_vector(63 downto 0) := ASYNC_CMD_ARRAY(6) & std_logic_vector(to_unsigned(134,32)); 
constant FRAMEWSIZE : std_logic_vector(63 downto 0) := ASYNC_CMD_ARRAY(7) & std_logic_vector(to_unsigned(341,32)); 

constant DEVICEINST_dev0 : std_logic_vector(191 downto 0) := ASYNC_CMD_ARRAY(8) & std_logic_vector(to_unsigned(2,32)) & 
																				  std_logic_vector(to_unsigned(134 ,32)) & std_logic_vector(to_unsigned(0,32)) & 
																				  std_logic_vector(to_unsigned(0,32)) & std_logic_vector(to_unsigned(0,32)); 
constant DEVICEINST_dev1 : std_logic_vector(191 downto 0) := ASYNC_CMD_ARRAY(8) & std_logic_vector(to_unsigned(2,32)) & 
                                                                                  std_logic_vector(to_unsigned(134 ,32)) & std_logic_vector(to_unsigned(0,32)) & 
                                                                                  std_logic_vector(to_unsigned(0,32)) & std_logic_vector(to_unsigned(0,32)); 
constant DEVICEINST_dev2 : std_logic_vector(191 downto 0) := ASYNC_CMD_ARRAY(8) & std_logic_vector(to_unsigned(3,32)) & 
																				  std_logic_vector(to_unsigned(18,32)) & std_logic_vector(to_unsigned(0,32)) & 
																				  std_logic_vector(to_unsigned(0,32)) & std_logic_vector(to_unsigned(0,32));

--A function to convert the memory blocks to async stream
function MEM_TO_24BYTE (
	memin : mem_type; wr_rd : std_logic; ack : std_logic)
return async_stream_type is 
	variable bytes24 : async_stream_type; 
	variable k : integer := 0; 
begin 
	if wr_rd = '0' and ack = '1' then --write and ack 
		bytes24(0) := ASYNC_CMD_ARRAY(0)(31 downto 24); 
		bytes24(1) := ASYNC_CMD_ARRAY(0)(23 downto 16); 
		bytes24(2) := ASYNC_CMD_ARRAY(0)(15 downto 8); 
		bytes24(3) := ASYNC_CMD_ARRAY(0)(7 downto 0); 
	elsif wr_rd = '0' and ack = '0' then --write and nack 
		bytes24(0) := ASYNC_CMD_ARRAY(1)(31 downto 24); 
		bytes24(1) := ASYNC_CMD_ARRAY(1)(23 downto 16); 
		bytes24(2) := ASYNC_CMD_ARRAY(1)(15 downto 8); 
		bytes24(3) := ASYNC_CMD_ARRAY(1)(7 downto 0); 	
	elsif wr_rd = '1' and ack = '1' then --read and ack 
		bytes24(0) := ASYNC_CMD_ARRAY(2)(31 downto 24); 
		bytes24(1) := ASYNC_CMD_ARRAY(2)(23 downto 16); 
		bytes24(2) := ASYNC_CMD_ARRAY(2)(15 downto 8); 
		bytes24(3) := ASYNC_CMD_ARRAY(2)(7 downto 0); 	
	elsif wr_rd = '1' and ack = '0' then --read and ack 
		bytes24(0) := ASYNC_CMD_ARRAY(3)(31 downto 24); 
		bytes24(1) := ASYNC_CMD_ARRAY(3)(23 downto 16); 
		bytes24(2) := ASYNC_CMD_ARRAY(3)(15 downto 8); 
		bytes24(3) := ASYNC_CMD_ARRAY(3)(7 downto 0); 	
	end if; 
	
	for k in 1 to HS_MEMARRAY_LENGTH loop 
		bytes24(k*4) := memin(k-1)(31 downto 24); 
		bytes24(k*4+1) := memin(k-1)(23 downto 16); 
		bytes24(k*4+2) := memin(k-1)(15 downto 8); 
		bytes24(k*4+3) := memin(k-1)(7 downto 0); 
	end loop;
	
return bytes24;
end MEM_TO_24BYTE; 

--a function to conver 192 length std_logic_vector to byte array
function VECTOR_TO_24BYTE (
	vecin : std_logic_vector(191 downto 0))
return  async_stream_type is
	variable bytes24 : async_stream_type;
	variable k : integer := 0; 
begin 
	for k in 23 downto 0 loop
		bytes24(23-k) := vecin(k*8+7 downto k*8);
	end loop;
return bytes24;
end VECTOR_TO_24BYTE; 

--a function to convert 64 length std_logic_vector to byte array
function VECTOR_TO_8BYTE (
	vecin : std_logic_vector(63 downto 0))
return  async_stream_type is
	variable bytes8 : async_stream_type;
	variable k : integer := 0; 
begin 
	for k in 7 downto 0 loop
		bytes8(7-k) := vecin(k*8+7 downto k*8);
	end loop;
	
	for k in 8 to 23 loop
		bytes8(k) := "00000000";
	end loop;
return bytes8;
end VECTOR_TO_8BYTE; 

signal DEVICEMAPACK_BYTE : async_stream_type := VECTOR_TO_8BYTE(DEVICEMAPACK);
signal FRAMERSIZE_BYTE : async_stream_type := VECTOR_TO_8BYTE(FRAMERSIZE);
signal FRAMEWSIZE_BYTE : async_stream_type := VECTOR_TO_8BYTE(FRAMEWSIZE); 
signal DEVICEINST_dev0_BYTE : async_stream_type := VECTOR_TO_24BYTE(DEVICEINST_dev0); 
signal DEVICEINST_dev1_BYTE : async_stream_type := VECTOR_TO_24BYTE(DEVICEINST_dev1); 
signal DEVICEINST_dev2_BYTE : async_stream_type := VECTOR_TO_24BYTE(DEVICEINST_dev2); 


type async_stream_type_array is array (0 to 5) of async_stream_type; 
signal pre_cobs_array : async_stream_type_array := (
	DEVICEMAPACK_BYTE, 
	FRAMERSIZE_BYTE,
	FRAMEWSIZE_BYTE,
	DEVICEINST_dev0_BYTE,
	DEVICEINST_dev1_BYTE,
	DEVICEINST_dev2_BYTE
	); 
	
type cobs_length_array_type is array (0 to 6) of std_logic_vector(4 downto 0); 
signal cobs_length_array : cobs_length_array_type := (
	std_logic_vector(to_unsigned(8, 5)),
	std_logic_vector(to_unsigned(8, 5)),
	std_logic_vector(to_unsigned(8, 5)),
	std_logic_vector(to_unsigned(24, 5)),
	std_logic_vector(to_unsigned(24, 5)),
	std_logic_vector(to_unsigned(24, 5)),
	std_logic_vector(to_unsigned(24, 5))
	);

signal cobs_begin : std_logic; 
signal cobs_conv_rdy : std_logic; 
signal array_cnt : unsigned(2 downto 0); 
signal pre_cobs_data : async_stream_type;
signal cobs_data, cobs_data_in : cobs_stream_types;
signal data_in_length, cobs_length, data_out_length : std_logic_vector(4 downto 0); 
signal fifo_push_cnt : unsigned(4 downto 0); 
signal conf_ack_flag : std_logic;

begin

sm_process: process(bus_clk, reset, dev_reset_in)
begin 
	if (reset = '1') then 
		async_sm <= IDLE; 
		array_cnt <= (others=>'0'); 
		fifo_push_cnt <= (others=>'0'); 
		cobs_begin <= '0'; 
		data_in_length <= (others=>'0'); 
		cobs_length <= (others=>'0'); 
		async_fifo_wr_data <= (others=>'0');
		async_fifo_wr_enb <= '0'; 
		conf_ack_flag <= '0'; 
		--array initilization 
		for i in 0 to 25 loop 
			cobs_data(i) <= (others=>'0'); 
		end loop;	
		for j in 0 to 23 loop 
			pre_cobs_data(j) <= (others=>'0'); 
		end loop;		
	elsif (rising_edge(bus_clk)) then 
		if dev_reset_in = '1' then --<-- this is a "synchronous reset" that puts the state machine in to RESET state, which then sets a DEVICE map once dev_reset_in is released
			async_sm <= DEVRESET;
			array_cnt <= (others=>'0'); 
			fifo_push_cnt <= (others=>'0'); 
			cobs_begin <= '0'; 
			data_in_length <= (others=>'0'); 
			cobs_length <= (others=>'0'); 
			async_fifo_wr_data <= (others=>'0');
			async_fifo_wr_enb <= '0'; 
			conf_ack_flag <= '0'; 
			--array initilization 
			for i in 0 to 25 loop 
				cobs_data(i) <= (others=>'0'); 
			end loop;	
			for j in 0 to 23 loop 
				pre_cobs_data(j) <= (others=>'0'); 
			end loop;	
		else 
			case async_sm is
				when IDLE => 
					if (conf_ack = '1') then --go to COBSCONV
						 async_sm <= COBSCONV; 
						 conf_ack_flag <= '1'; 
						 pre_cobs_data <= MEM_TO_24BYTE(conf_mem_in, '0', '1');						  
					else 
						 async_sm <= IDLE; 
					end if; 
					cobs_begin <= '0';
					async_fifo_wr_enb <= '0'; 
					async_fifo_wr_data <= (others=>'0'); 
				when DEVRESET => 
					--currently just go directly to the next state 
					async_sm <= COBSCONV; 
					cobs_begin <= '0';
				when COBSCONV =>		
					if conf_ack_flag = '1' then 
						data_in_length <= std_logic_vector(to_unsigned(24, 5));
					else
						pre_cobs_data <= pre_cobs_array(to_integer(array_cnt)); 
						data_in_length <= cobs_length_array(to_integer(array_cnt));
					end if; 
					cobs_begin <= '1';
					async_fifo_wr_enb <= '0';
					async_sm <= COBSWAIT; 				
				when COBSWAIT => 
					cobs_begin <= '0';
					if cobs_conv_rdy = '1' then --wait for cobs to finish conversion. 
						async_sm <= COBSPUSH; 
						cobs_data <= cobs_data_in;
						cobs_length <= data_out_length;
						cobs_begin <= '0';
					end if; 
				when COBSPUSH => --push COBS to FIFO
					cobs_begin <= '0';
					if (fifo_push_cnt >= unsigned(cobs_length) - 1) then 
						fifo_push_cnt <= (others=>'0'); 
						if conf_ack_flag = '1' then 
							async_sm <= IDLE;
							conf_ack_flag <= '0'; 
						else 
							if array_cnt < 5 then
								array_cnt <= array_cnt + 1; 
								async_sm <= COBSCONV; 
							else 
								array_cnt <= (others=>'0'); 
								async_sm <= IDLE;
							end if;
						end if; 
					else 
						fifo_push_cnt <= fifo_push_cnt + 1; 
					end if; 
					
					async_fifo_wr_data <= cobs_data(to_integer(fifo_push_cnt)); 
					async_fifo_wr_enb <= '1'; 
			end case; 
		end if; 
	end if; 
end process; 

COBS_inst: cobs_encoder  
	port map(	
		 bus_clk => bus_clk, 
		 reset => reset, 
		 --cobs inputs
		 pre_cobs_data_in => pre_cobs_data, 
		 data_in_length => data_in_length, 
		 cobs_conv_begin => cobs_begin,
		 --cobs outputs
		 cobs_data_out => cobs_data_in,
		 data_out_length => data_out_length,
		 cobs_conv_rdy => cobs_conv_rdy
	);

end Behavioral;
