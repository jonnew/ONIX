--serialized communication scheduler
--Jie Zhang, MWL MIT 
--Description: This headstage scheduler takes an array of 16 bits data and converts into a 12 bits stream for the serilizer
--The length of the 16bits array depends on the number of devices avaliable on the headstage. 

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all; 
use work.myDeclare.all;

entity com_scheduler is
  Port (
    clk : in std_logic; 
    reset : in std_logic; 
    device_data_array : in device_data_array_type; -- Array of dimension (No. of device x 16bits) data, each 16bits corresponds to 1 device input stream
	 device_flag_array : in std_logic_vector(0 to NUMBEROFDEVICE-1);  
    serdes_data_out : out std_logic_vector(11 downto 0);
    serdes_valid_out : out std_logic
   );
end com_scheduler;

architecture Behavioral of com_scheduler is

--state machines
type schstate_type is (IDLE, CONV_P1, CONV_P2, CONV_P3, CONV_P4); --state machine definition
signal schstate : schstate_type; 

signal sample_cnt : unsigned(LOG2MAXSAMPLES-1 downto 0); --keeping track of the current number of sample.  
signal device_id_cnt : unsigned(LOG2NUMBEROFDEVICE-1 downto 0); --keeping track of the current device.
signal data_reminder : std_logic_vector(11 downto 0); 
signal serdes_data : std_logic_vector(11 downto 0); 
signal serdes_valid : std_logic; 

begin

--output mapping
serdes_data_out <= serdes_data; 
serdes_valid_out <= serdes_valid;

sch_proc : process(clk, reset) 
begin 
    if (reset = '1') then 
        schstate <= IDLE;
		  sample_cnt <= (others=>'0'); 
		  device_id_cnt <= (others=>'0'); 
		  data_reminder <= (others=>'0'); 
		  serdes_data <= (others=>'0'); 
		  serdes_valid <= '0'; 
    elsif (rising_edge(clk)) then 
        case schstate is 
            when IDLE => 
					if device_flag_array(0) = '1' then 
						schstate <= CONV_P1; 
					else 
						schstate <= IDLE; 
					end if; 
					device_id_cnt <= (others=>'0'); 
					sample_cnt <= (others=>'0'); 
					serdes_valid <= '0';
            when CONV_P1 => --16 into 12 with 4 as reminder 
					if device_id_cnt >= NUMBEROFDEVICE-1 then --if device id reach the max then go to IDLE state 
						schstate <= IDLE; 
					else 
						if device_flag_array(to_integer(device_id_cnt)) = '1' then --check device data avaliable flag 
							if sample_cnt <= data_length_array(to_integer(device_id_cnt)) then --there is still samples 
								sample_cnt <= sample_cnt + 1; 
							else 
								sample_cnt <= (others=>'0');
								device_id_cnt <= device_id_cnt + 1; 
							end if; 
							schstate <= CONV_P2; 
							------ Take new data / rise valie flag ------ 
							data_reminder(3 downto 0) <=  device_data_array(to_integer(device_id_cnt))(3 downto 0); 
							serdes_data <= device_data_array(to_integer(device_id_cnt))(15 downto 4);
							serdes_valid <= '1'; 
							---------------------------------------------
						else --halt and wait for device data avaliable flag to go high
							schstate <= CONV_P1; 
							serdes_valid <= '0';
						end if; 
					end if; 				
            when CONV_P2 => --4+16 into 12 with 8 as reminder 
					if device_id_cnt >= NUMBEROFDEVICE-1 then --if device id reach the max then go to IDLE state 
						schstate <= IDLE; --here need to send the left over 4 bits 
						serdes_data(11 downto 8) <=  data_reminder(3 downto 0);
						serdes_data(7 downto 0) <= (others=>'0'); 
						serdes_valid <= '1'; 
					else 
						if device_flag_array(to_integer(device_id_cnt)) = '1' then --check device data avaliable flag 
							if sample_cnt <= data_length_array(to_integer(device_id_cnt)) then --there is still samples 
								sample_cnt <= sample_cnt + 1; 
							else 
								sample_cnt <= sample_cnt + 1; 
								device_id_cnt <= device_id_cnt + 1; 
							end if; 
							schstate <= CONV_P3; 
							------ Take new data / rise valie flag ------ 
							data_reminder(7 downto 0) <=  device_data_array(to_integer(device_id_cnt))(7 downto 0); 
							serdes_data <= device_data_array(to_integer(device_id_cnt))(15 downto 8) & data_reminder(3 downto 0);
							serdes_valid <= '1'; 
							---------------------------------------------
						else --halt and wait for device data avaliable flag to go high
							schstate <= CONV_P2; 
						end if; 
					end if; 							
            when CONV_P3 => --8+16 into 12 with 12 as reminder 
					if device_id_cnt >= NUMBEROFDEVICE-1 then --if device id reach the max then go to IDLE state  
						schstate <= IDLE; --here need to send the left over 8 bits 
						serdes_data(11 downto 4) <=  data_reminder(7 downto 0);
						serdes_data(3 downto 0) <= (others=>'0'); 
						serdes_valid <= '1'; 
					else 
						if device_flag_array(to_integer(device_id_cnt)) = '1' then --check device data avaliable flag 
							if sample_cnt <= data_length_array(to_integer(device_id_cnt)) then --there is still samples 
								sample_cnt <= sample_cnt + 1; 
							else 
								sample_cnt <= sample_cnt + 1; 
								device_id_cnt <= device_id_cnt + 1; 
							end if; 
							schstate <= CONV_P4; 
							------ Take new data / rise valie flag ------ 
							data_reminder(11 downto 0) <=  device_data_array(to_integer(device_id_cnt))(11 downto 0); 
							serdes_data <= device_data_array(to_integer(device_id_cnt))(15 downto 12) & data_reminder(7 downto 0);
							serdes_valid <= '1'; 
							---------------------------------------------
						else --halt and wait for device data avaliable flag to go high
							schstate <= CONV_P3;
						end if; 
					end if; 			
            when CONV_P4 => --12 into 12 with 0 as reminder 
					if device_id_cnt >= NUMBEROFDEVICE-1 then --if device id reach the max then go to IDLE state 
						schstate <= IDLE; 
					else 
						schstate <= CONV_P1;
					end if; 
					--Do not need to check if the flag is high because we don't take new data at this state 
					--We also do not need to increase the sample cnt and deivce_id_cnt 
					------ send the data in the reminder registers ------ 
					data_reminder <= (others=>'0'); 
					serdes_data <= data_reminder(11 downto 0); 
					serdes_valid <= '1'; 
					-----------------------------------------------------			
        end case; 
    end if; 
end process; 


end Behavioral;
