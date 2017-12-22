library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.myDeclare.all;

entity xillydemo is
  port (
    PCIE_PERST_B_LS : IN std_logic;
    PCIE_REFCLK_N : IN std_logic;
    PCIE_REFCLK_P : IN std_logic;
    PCIE_RX_N : IN std_logic_vector(7 DOWNTO 0);
    PCIE_RX_P : IN std_logic_vector(7 DOWNTO 0);
    GPIO_LED : OUT std_logic_vector(3 DOWNTO 0);
    PCIE_TX_N : OUT std_logic_vector(7 DOWNTO 0);
    PCIE_TX_P : OUT std_logic_vector(7 DOWNTO 0));
end xillydemo;

architecture sample_arch of xillydemo is

  component xillybus
  port (
    PCIE_PERST_B_LS : IN std_logic;
    PCIE_REFCLK_N : IN std_logic;
    PCIE_REFCLK_P : IN std_logic;
    PCIE_RX_N : IN std_logic_vector(7 DOWNTO 0);
    PCIE_RX_P : IN std_logic_vector(7 DOWNTO 0);
    GPIO_LED : OUT std_logic_vector(3 DOWNTO 0);
    PCIE_TX_N : OUT std_logic_vector(7 DOWNTO 0);
    PCIE_TX_P : OUT std_logic_vector(7 DOWNTO 0);
    bus_clk : OUT std_logic;
    quiesce : OUT std_logic;
    user_r_async_read_8_rden : OUT std_logic;
    user_r_async_read_8_empty : IN std_logic;
    user_r_async_read_8_data : IN std_logic_vector(7 DOWNTO 0);
    user_r_async_read_8_eof : IN std_logic;
    user_r_async_read_8_open : OUT std_logic;
    user_r_cmd_mem_32_rden : OUT std_logic;
    user_r_cmd_mem_32_empty : IN std_logic;
    user_r_cmd_mem_32_data : IN std_logic_vector(31 DOWNTO 0);
    user_r_cmd_mem_32_eof : IN std_logic;
    user_r_cmd_mem_32_open : OUT std_logic;
    user_w_cmd_mem_32_wren : OUT std_logic;
    user_w_cmd_mem_32_full : IN std_logic;
    user_w_cmd_mem_32_data : OUT std_logic_vector(31 DOWNTO 0);
    user_w_cmd_mem_32_open : OUT std_logic;
    user_cmd_mem_32_addr : OUT std_logic_vector(4 DOWNTO 0);
    user_cmd_mem_32_addr_update : OUT std_logic;
    user_r_data_read_32_rden : OUT std_logic;
    user_r_data_read_32_empty : IN std_logic;
    user_r_data_read_32_data : IN std_logic_vector(31 DOWNTO 0);
    user_r_data_read_32_eof : IN std_logic;
    user_r_data_read_32_open : OUT std_logic);
end component;

  component fifo_xillybus_8
    port (
      clk: IN std_logic;
      srst: IN std_logic;
      din: IN std_logic_VECTOR(7 downto 0);
      wr_en: IN std_logic;
      rd_en: IN std_logic;
      dout: OUT std_logic_VECTOR(7 downto 0);
      full: OUT std_logic;
      empty: OUT std_logic);
  end component;

    COMPONENT fifo_xillybus_32
      PORT (
          rst : IN STD_LOGIC;
          wr_clk : IN STD_LOGIC;
          rd_clk : IN STD_LOGIC;
          din : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
          wr_en : IN STD_LOGIC;
          rd_en : IN STD_LOGIC;
          dout : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
          full : OUT STD_LOGIC;
          empty : OUT STD_LOGIC
        );
    END COMPONENT;
  
  --asynchronous communication channel controller 
    component async_com_control is
      port (
           bus_clk : in std_logic; 
           reset : in std_logic;
          --pclk  : in std_logic; 
          --din : in std_logic_vector(11 downto 0); --headstage communication input from the Deserilizer 
           dev_reset_in : in std_logic; --a temp debug signal to mimic the communication via magic number to the headstage 
           conf_ack : in std_logic; 
           conf_nack : in std_logic; 
           conf_done : in std_logic;
           conf_mem_in : in mem_type;
           --cobs fifo output 
           async_fifo_wr_enb : out std_logic; 
           async_fifo_wr_data : out std_logic_vector(7 downto 0)
          );
      end component;
   
   --configuration memory controller    
    component mem_conf_control is
      port (
          bus_clk : in std_logic; 
          reset : in std_logic; 
          user_mem_32_addr : in std_logic_vector(3 downto 0);
          user_w_mem_32_wren : in std_logic; 
          user_r_mem_32_rden : in std_logic; 
          user_w_mem_32_data : in std_logic_vector(31 downto 0);
          user_r_mem_32_data : out std_logic_vector(31 downto 0); 
          dev_reset_out : out std_logic;
          conf_ack : out std_logic;
          conf_nack : out std_logic;
          mem_out : out mem_type
      );
      end component;    
      
      component hs_com_control is
      port (
          bus_clk : in std_logic; 
          global_reset : in std_logic; 
          dev_reset_in : in std_logic; 
          hs_com_fifo_data : out std_logic_vector(31 downto 0); 
          hs_com_fifo_enb : out std_logic
      );
      end component;
      
  --a simple clock divider 
  component clk_div is
      generic (MAXD: natural:=5);
      port(
           clk: in std_logic;
           reset: in std_logic;
           div: in integer range 0 to MAXD;
           div_clk: out std_logic 
           );
  end component;
      

-- Synplicity black box declaration
  attribute syn_black_box : boolean;
  attribute syn_black_box of fifo_xillybus_32: component is true;
  attribute syn_black_box of fifo_xillybus_8: component is true;

  type demo_mem is array(0 TO 31) of std_logic_vector(7 DOWNTO 0);
  signal demoarray : demo_mem;
  
  signal reset_8 : std_logic;
  signal reset_32 : std_logic;
  signal conf_ack, conf_nack : std_logic; 
  signal conf_mem : mem_type;
  signal hs_com_fifo_data : std_logic_vector(31 downto 0); 
  signal hs_com_fifo_enb : std_logic; 
  
  signal bus_clk :  std_logic;
  signal quiesce : std_logic;
  signal user_r_async_read_8_rden :  std_logic;
  signal user_r_async_read_8_empty :  std_logic;
  signal user_r_async_read_8_data :  std_logic_vector(7 DOWNTO 0);
  signal user_r_async_read_8_eof :  std_logic;
  signal user_r_async_read_8_open :  std_logic;
  signal user_r_cmd_mem_32_rden :  std_logic;
  signal user_r_cmd_mem_32_empty :  std_logic;
  signal user_r_cmd_mem_32_data :  std_logic_vector(31 DOWNTO 0);
  signal user_r_cmd_mem_32_eof :  std_logic;
  signal user_r_cmd_mem_32_open :  std_logic;
  signal user_w_cmd_mem_32_wren :  std_logic;
  signal user_w_cmd_mem_32_full :  std_logic;
  signal user_w_cmd_mem_32_data :  std_logic_vector(31 DOWNTO 0);
  signal user_w_cmd_mem_32_open :  std_logic;
  signal user_cmd_mem_32_addr :  std_logic_vector(4 DOWNTO 0);
  signal user_cmd_mem_32_addr_update :  std_logic;
  signal user_r_data_read_32_rden :  std_logic;
  signal user_r_data_read_32_empty :  std_logic;
  signal user_r_data_read_32_data :  std_logic_vector(31 DOWNTO 0);
  signal user_r_data_read_32_eof :  std_logic;
  signal user_r_data_read_32_open :  std_logic;
  
  signal async_fifo_wr_enb : std_logic; 
  signal async_fifo_wr_data : std_logic_vector(7 downto 0);
  signal clk0p5Hz : std_logic; 
  signal clk_slow : std_logic; 
  signal dev_reset : std_logic; 
  signal hs_com_reset : std_logic;

begin
  xillybus_ins : xillybus
  port map (
    -- Ports related to /dev/xillybus_async_read_8
    -- FPGA to CPU signals:
    user_r_async_read_8_rden => user_r_async_read_8_rden,
    user_r_async_read_8_empty => user_r_async_read_8_empty,
    user_r_async_read_8_data => user_r_async_read_8_data,
    user_r_async_read_8_eof => user_r_async_read_8_eof,
    user_r_async_read_8_open => user_r_async_read_8_open,

    -- Ports related to /dev/xillybus_cmd_mem_32
    -- FPGA to CPU signals:
    user_r_cmd_mem_32_rden => user_r_cmd_mem_32_rden,
    user_r_cmd_mem_32_empty => user_r_cmd_mem_32_empty,
    user_r_cmd_mem_32_data => user_r_cmd_mem_32_data,
    user_r_cmd_mem_32_eof => user_r_cmd_mem_32_eof,
    user_r_cmd_mem_32_open => user_r_cmd_mem_32_open,
    -- CPU to FPGA signals:
    user_w_cmd_mem_32_wren => user_w_cmd_mem_32_wren,
    user_w_cmd_mem_32_full => user_w_cmd_mem_32_full,
    user_w_cmd_mem_32_data => user_w_cmd_mem_32_data,
    user_w_cmd_mem_32_open => user_w_cmd_mem_32_open,
    -- Address signals:
    user_cmd_mem_32_addr => user_cmd_mem_32_addr,
    user_cmd_mem_32_addr_update => user_cmd_mem_32_addr_update,

    -- Ports related to /dev/xillybus_data_read_32
    -- FPGA to CPU signals:
    user_r_data_read_32_rden => user_r_data_read_32_rden,
    user_r_data_read_32_empty => user_r_data_read_32_empty,
    user_r_data_read_32_data => user_r_data_read_32_data,
    user_r_data_read_32_eof => user_r_data_read_32_eof,
    user_r_data_read_32_open => user_r_data_read_32_open,

    -- General signals
    PCIE_PERST_B_LS => PCIE_PERST_B_LS,
    PCIE_REFCLK_N => PCIE_REFCLK_N,
    PCIE_REFCLK_P => PCIE_REFCLK_P,
    PCIE_RX_N => PCIE_RX_N,
    PCIE_RX_P => PCIE_RX_P,
    GPIO_LED => GPIO_LED,
    PCIE_TX_N => PCIE_TX_N,
    PCIE_TX_P => PCIE_TX_P,
    bus_clk => bus_clk,
    quiesce => quiesce
);
      

highspeed_data: hs_com_control
port map(
          bus_clk => bus_clk,  
          global_reset => quiesce,
            --device_num : in std_logic_vector(LOG2_MAX_DEVICE_NUMBER-1 downto 0); 
          dev_reset_in => hs_com_reset,
          hs_com_fifo_data => hs_com_fifo_data,  
          hs_com_fifo_enb => hs_com_fifo_enb
); 

hs_com_reset <= dev_reset or reset_32;

--  32bit uni-directional data bus to the host
  fifo_32 : fifo_xillybus_32
    port map(
      wr_clk    => bus_clk,
      rd_clk    => bus_clk,
      rst       => hs_com_reset,
      din        => hs_com_fifo_data,
      wr_en      => hs_com_fifo_enb,
      rd_en      => user_r_data_read_32_rden,
      dout       => user_r_data_read_32_data,
      full       => open,
      empty      => user_r_data_read_32_empty
      );
      
  reset_32 <= not (user_r_data_read_32_open);
  user_r_data_read_32_eof <= '0';
  
  
--  8-bit loopback
  fifo_8 : fifo_xillybus_8
    port map(
      clk        => bus_clk,
      srst       => reset_8,
      din        => async_fifo_wr_data,--async_fifo_wr_data,
      wr_en      => async_fifo_wr_enb,
      rd_en      => user_r_async_read_8_rden,
      dout       => user_r_async_read_8_data,
      full       => open,
      empty      => user_r_async_read_8_empty
      );

    reset_8 <= not (user_r_async_read_8_open);
    user_r_async_read_8_eof <= '0';
  
-- Async communication controller 
     async_communication: async_com_control PORT MAP (
           bus_clk => bus_clk,
           reset => reset_8,
           dev_reset_in => dev_reset,
           conf_ack => conf_ack,
           conf_nack => '1',
           conf_done => '1',
           conf_mem_in => conf_mem,
           async_fifo_wr_enb => async_fifo_wr_enb,
           async_fifo_wr_data => async_fifo_wr_data
         );  
      
     mem_controller: mem_conf_control PORT MAP (
       bus_clk => bus_clk,
       reset => quiesce,
       user_mem_32_addr => user_cmd_mem_32_addr(3 downto 0),
       user_w_mem_32_wren => user_w_cmd_mem_32_wren,
       user_r_mem_32_rden => user_r_cmd_mem_32_rden,
       user_w_mem_32_data => user_w_cmd_mem_32_data,
       user_r_mem_32_data => user_r_cmd_mem_32_data,
       dev_reset_out => dev_reset,
       conf_ack => conf_ack,
       conf_nack => conf_nack,
       mem_out => conf_mem
     );
             
      clk_div_0p5Hz: clk_div generic map (MAXD => 500_000_000)
      port map ( clk => bus_clk, reset => quiesce, div => 500_000_000, div_clk => clk0p5Hz);
  
  
end sample_arch;
