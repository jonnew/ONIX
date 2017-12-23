
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

package myDeclare is


constant MEMARRAYLENGTH : integer := 11; 
constant HS_MEMARRAY_LENGTH : integer := 5; 
constant MAXDEVICENUMBER : integer := 16; 
constant LOG2_MAX_DEVICE_NUMBER : integer := 4; 
constant LOG2_MAX_DATA_FRAME_PER_DEVICE: integer := 10;

--memory register blocks
type mem_type is array (0 to MEMARRAYLENGTH-1) of std_logic_vector(31 downto 0); 
type async_stream_type is array (0 to 23) of std_logic_vector(7 downto 0); 
type cobs_stream_types is array (0 to 25) of std_logic_vector(7 downto 0); 

--constants below are used at the headstage only 
constant NUMBEROFDEVICE : integer := 3;
constant LOG2NUMBEROFDEVICE : integer := 2;
type data_length_array_type is array (0 to NUMBEROFDEVICE-1) of integer; 
constant data_length_array : data_length_array_type := (67, 67, 9); --this is in units of 32bits  
constant MAXSAMPLES : integer := 10; 
constant LOG2MAXSAMPLES : integer := 4;
type device_data_array_type is array (0 to NUMBEROFDEVICE-1) of std_logic_vector(15 downto 0); 

--FIFO declarations 
COMPONENT fifo_imu_16bits
  PORT (
    rst : IN STD_LOGIC;
    wr_clk : IN STD_LOGIC;
    rd_clk : IN STD_LOGIC;
    din : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    wr_en : IN STD_LOGIC;
    rd_en : IN STD_LOGIC;
    dout : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    full : OUT STD_LOGIC;
    empty : OUT STD_LOGIC;
    prog_empty : OUT STD_LOGIC
  );
END COMPONENT;

COMPONENT fifo_intan_16bits
  PORT (
    rst : IN STD_LOGIC;
    wr_clk : IN STD_LOGIC;
    rd_clk : IN STD_LOGIC;
    din : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    wr_en : IN STD_LOGIC;
    rd_en : IN STD_LOGIC;
    dout : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    full : OUT STD_LOGIC;
    empty : OUT STD_LOGIC;
    prog_empty : OUT STD_LOGIC;
    wr_rst_busy : OUT STD_LOGIC;
    rd_rst_busy : OUT STD_LOGIC
  );
END COMPONENT;



end myDeclare;

package body myDeclare is

 
end myDeclare;
