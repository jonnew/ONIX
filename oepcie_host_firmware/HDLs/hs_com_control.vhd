--hs_com_control.vdh
--Jie Zhang, MWL, MIT.
--This module handles the multi-sensor buffering and multiplexing to the xillybus 32bits data FIFO
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;
use work.myDeclare.all;

entity hs_com_control is
port (
	bus_clk : in std_logic; 
	global_reset : in std_logic; 
	--device_num : in std_logic_vector(LOG2_MAX_DEVICE_NUMBER-1 downto 0); 
	dev_reset_in : in std_logic; 
	hs_com_fifo_data : out std_logic_vector(31 downto 0); 
	hs_com_fifo_enb : out std_logic
);
end hs_com_control;

architecture Behavioral of hs_com_control is

type hscomstate_type is (PAUSE, LOOPCHECK, HEADER, DEVICEMAPINDEX, DEVICEPUSH, NEWDEVICE); --state machine definition
signal hscomstate : hscomstate_type; 
signal dev_cnt : unsigned(LOG2_MAX_DEVICE_NUMBER-1 downto 0); --allocate a device counter bounded by MAX DEVICE NUMBER 
signal data_cnt : unsigned(LOG2_MAX_DATA_FRAME_PER_DEVICE-1 downto 0); --allocate a data counter 
signal data_toggle : std_logic := '0'; --used to merge 16bits data to 32bits bus 
signal clk_slow : std_logic := '0'; 

type intan_data_type is array (0 to 66) of std_logic_vector(15 downto 0);
type intan_device_array_type is array (0 to 3) of intan_data_type; 
signal intan_device_array : intan_device_array_type;  
signal frame_number : unsigned(63 downto 0); 

--sensor specific signals 
signal threshold, threshold_buf, sensor_clk, sensor_rd, sensor_wr : std_logic_vector(NUMBEROFDEVICE-1 downto 0); 
type sensor_data_array_type is array (0 to NUMBEROFDEVICE-1) of std_logic_vector(15 downto 0); 
signal sensor_data_in_array :  sensor_data_array_type; 
signal sensor_data_out_array : sensor_data_array_type; 

  component clk_div is
      generic (MAXD: natural:=5);
      port(
           clk: in std_logic;
           reset: in std_logic;
           div: in integer range 0 to MAXD;
           div_clk: out std_logic 
           );
  end component;

begin

clk_div_slow: clk_div generic map (MAXD => 125)
      port map ( clk => bus_clk, reset => global_reset, div => 125, div_clk => clk_slow);

clk_div_intan_0: clk_div generic map (MAXD => 2)
      port map ( clk => clk_slow, reset => global_reset, div => 2, div_clk => sensor_clk(0));

clk_div_intan_1: clk_div generic map (MAXD => 7)
      port map ( clk => clk_slow, reset => global_reset, div => 5, div_clk => sensor_clk(1));
      
clk_div_imu_2: clk_div generic map (MAXD => 2000)
            port map ( clk => clk_slow, reset => global_reset, div => 2000, div_clk => sensor_clk(2));

--some fake intan operations: 
intan_proc: process(clk_slow, global_reset, hscomstate) 
begin 
	if (global_reset = '1') then 
		for i in 0 to 3 loop
			for j in 0 to 66 loop 
				intan_device_array(i)(j) <= (others=>'0'); 
			end loop;
		end loop; 
	elsif (rising_edge(clk_slow)) then 
		if hscomstate = NEWDEVICE then 
			for i in 0 to 3 loop
				for j in 0 to 66 loop 
					intan_device_array(i)(j) <= std_logic_vector(unsigned(intan_device_array(i)(j)) + j);
				end loop;
			end loop;
		end if; 
	end if; 
end process; 

--instantiate sensor FIFOs 
--  32bit uni-directional data bus to the host
  fifo_intan_inst_0 : fifo_intan_16bits
    port map(
      wr_clk    => sensor_clk(0),
      rd_clk    => bus_clk,
      rst       => '0',
      din        => (others=>'0'),
      wr_en      => '1',
      rd_en      => sensor_rd(0),
      dout       => sensor_data_out_array(0),
      full       => open,
      empty      => open,
      prog_empty => threshold(0)
      );
      
   fifo_intan_inst_1 : fifo_intan_16bits
     port map(
      wr_clk    => sensor_clk(1),
      rd_clk    => bus_clk,
      rst       => '0',
      din        => (others=>'0'),
      wr_en      => '1',
      rd_en      => sensor_rd(1),
      dout       => sensor_data_out_array(1),
      full       => open,
      empty      => open,
      prog_empty => threshold(1)
      );

   fifo_imu_inst_0 : fifo_imu_16bits
     port map(
      wr_clk    => sensor_clk(2),
      rd_clk    => bus_clk,
      rst       => '0',
      din        => (others=>'0'),
      wr_en      => '1',
      rd_en      => sensor_rd(2),
      dout       => sensor_data_out_array(2),
      full       => open,
      empty      => open,
      prog_empty => threshold(2)
      );      

--hs com state machine

hs_com_proc: process(bus_clk, global_reset, data_cnt, threshold_buf, threshold, sensor_clk, sensor_rd) 
    begin 
        if (dev_reset_in = '1' or global_reset = '1') then 
            hscomstate <= PAUSE;
            dev_cnt <= (others=>'0'); 
            data_cnt <= (others=>'0'); 
            data_toggle <= '0'; 
            hs_com_fifo_data <= (others=>'0'); 
            hs_com_fifo_enb <= '0';
            frame_number <= (others=>'0');
            threshold_buf <= (others=>'1');
            sensor_rd <= (others=>'0'); 
        elsif (rising_edge(bus_clk)) then 
            case hscomstate is 
                    when PAUSE => 
                        if data_cnt >= 500 then --pause 500 cycles
                            hscomstate <= LOOPCHECK;
                            threshold_buf <= threshold;
                            data_cnt <= (others=>'0'); 
                        else 
                            data_cnt <= data_cnt + 1; 
                        end if; 
                        hs_com_fifo_data <= (others=>'0'); 
                        hs_com_fifo_enb <= '0';
                        threshold_buf <= (others=>'1'); 
                        sensor_rd <= (others=>'0'); 
                        dev_cnt <= (others=>'0'); 
                    when LOOPCHECK => 
                        hs_com_fifo_enb <= '0'; 
                        data_toggle <= '0';
                        sensor_rd <= (others=>'0'); 
                        --count the number of devices that is not zero. 
                        if data_cnt > NUMBEROFDEVICE-1 then 
                            if dev_cnt > 0 then 
                                hscomstate <= HEADER; 
                            else 
                                hscomstate <= LOOPCHECK;
                                dev_cnt <= (others=>'0');
                                threshold_buf <= threshold; 
                            end if; 
                            data_cnt <= (others=>'0'); 
                        else 
                            if (threshold_buf(to_integer(data_cnt)) = '0') then 
                                dev_cnt <= dev_cnt + 1; 
                            end if;  
                            hscomstate <= LOOPCHECK;
                            data_cnt <= data_cnt + 1; 
                        end if; 
                    when HEADER =>
                        if data_cnt = 0 then --send first 32bits of frame number (bytes 0-3)
                            hs_com_fifo_data <= std_logic_vector(frame_number(31 downto 0)); 
                            --hs_com_fifo_data <= x"01_23_45_67";
                            --hs_com_fifo_data <= x"00_00_00_01";
                        elsif data_cnt = 1 then --send the rest 32bits of frame number (bytes 4-7)
                            --hs_com_fifo_data <= x"00_00_00_00";
                            hs_com_fifo_data <=  std_logic_vector(frame_number(63 downto 32));
                        elsif data_cnt = 2 then 
                            hs_com_fifo_data(15 downto 0) <= std_logic_vector(to_unsigned(to_integer(dev_cnt), 16)); --3 devices in this frame
                            hs_com_fifo_data(23 downto 16) <= (others=>'0'); --corrupt
                            hs_com_fifo_data(31 downto 24) <= (others=>'0'); --reserved
                        else --reserved
                            hs_com_fifo_data <= (others=>'0'); 
                        end if; 
                        hs_com_fifo_enb <= '1'; 
                        
                        if data_cnt >= 7 then 
                            data_cnt <= (others=>'0'); 
                            hscomstate <= DEVICEMAPINDEX; 
                            dev_cnt <= (others=>'0'); 
                        else 
                            data_cnt <= data_cnt + 1; 
                            hscomstate <= HEADER; 
                        end if; 
                    when DEVICEMAPINDEX => 
                        if dev_cnt >= NUMBEROFDEVICE-1 then --3 device 
                           dev_cnt <= (others=>'0'); 
                           hscomstate <= NEWDEVICE;
                        else
                           dev_cnt <= dev_cnt + 1; 
                           hscomstate <= DEVICEMAPINDEX;
                        end if; 
                        
                        if (threshold_buf(to_integer(dev_cnt)) = '0') then --almost empty flag not asserted  
                            hs_com_fifo_data <= std_logic_vector(to_unsigned(to_integer(dev_cnt),32));
                            hs_com_fifo_enb <= '1'; 
                        else 
                            hs_com_fifo_enb <= '0'; 
                        end if; 
                                           
                    when DEVICEPUSH => 

                           if data_cnt >= data_length_array(to_integer(dev_cnt))-1 then 
                                hscomstate <= NEWDEVICE;
                                dev_cnt <= dev_cnt + 1; 
                                sensor_rd(to_integer(dev_cnt)) <= '0';
                           else  
                                data_cnt <= data_cnt + 1; 
                                sensor_rd(to_integer(dev_cnt)) <= '1';
                           end if;
                           
                           data_toggle <= not data_toggle; 
                           
                           if data_toggle = '0' then 
                              hs_com_fifo_data(15 downto 0) <= intan_device_array(to_integer(dev_cnt))(to_integer(data_cnt)); 
                              hs_com_fifo_enb <= '0'; 
                           else 
                              hs_com_fifo_data(31 downto 16) <= intan_device_array(to_integer(dev_cnt))(to_integer(data_cnt)); 
                              hs_com_fifo_enb <= '1'; 
                           end if;
                        
                    when NEWDEVICE =>
                    
                        if (dev_cnt > NUMBEROFDEVICE-1) then 
                            if data_toggle = '1' then --needs padding 
                               hs_com_fifo_data(31 downto 16) <= (others=>'0'); 
                               hs_com_fifo_enb <= '1'; 
                            else
                               hs_com_fifo_enb <= '0'; 
                            end if; 
                            hscomstate <= LOOPCHECK;
                            frame_number <= frame_number + 1; 
                            dev_cnt <= (others=>'0');
                            threshold_buf <= threshold;                        
                        else 
                            if threshold_buf(to_integer(dev_cnt)) = '0' then --asserted  
                                sensor_rd(to_integer(dev_cnt)) <= '1';
                                hscomstate <= DEVICEPUSH; 
                            else 
                                dev_cnt <= dev_cnt + 1; 
                                hscomstate <= NEWDEVICE; 
                            end if;  
                            hs_com_fifo_enb <= '0'; 
                        end if; 
                        data_cnt <= (others=>'0'); 
                        
            end case; 
	end if; 
end process; 


end Behavioral;

