----------------------------------------------------------------------------------
--This is adapted from this code: https://eewiki.net/pages/viewpage.action?pageId=10125324
--modification: now sends 3 I2C packets with only a single ena signal. [device id | register address | value]
--by: Jie (Jack) Zhang MWL-MIT
----------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

ENTITY i2c_master IS
  GENERIC(
    input_clk : INTEGER := 50_000_000; --input clock speed from user logic in Hz
    bus_clk   : INTEGER := 500_000);   --speed the i2c bus (scl) will run at in Hz
  PORT(
    clk       : IN     STD_LOGIC;                    --system clock
    reset   : IN     STD_LOGIC;                    --active high reset
    ena       : IN     STD_LOGIC;                    --latch in command
    devid     : IN     STD_LOGIC_VECTOR(6 DOWNTO 0); --device id of target slave
	 addr      : in     std_logic_vector(7 downto 0); 
    rw        : IN     STD_LOGIC;                    --'0' is write, '1' is read
    data_wr   : IN     STD_LOGIC_VECTOR(7 DOWNTO 0); --data to write to slave
    busy      : OUT    STD_LOGIC;                    --indicates transaction in progress
    data_rd   : OUT    STD_LOGIC_VECTOR(7 DOWNTO 0); --data read from slave
    ack_error : BUFFER STD_LOGIC;                    --flag if improper acknowledge from slave
    sda       : INOUT  STD_LOGIC;                    --serial data output of i2c bus
    scl       : INOUT  STD_LOGIC);                   --serial clock output of i2c bus
END i2c_master;

ARCHITECTURE Behavioral OF i2c_master IS

  CONSTANT divider  :  INTEGER := (input_clk/bus_clk)/4; --number of clocks in 1/4 cycle of scl
  TYPE machine IS(ready, start, command, slv_ack1, wr, rd, slv_ack2, mstr_ack, stop); --needed states
  SIGNAL state         : machine;                        --state machine
  SIGNAL data_clk      : STD_LOGIC;                      --data clock for sda
  SIGNAL data_clk_prev : STD_LOGIC;                      --data clock during previous system clock
  SIGNAL scl_clk       : STD_LOGIC;                      --constantly running internal scl
  SIGNAL scl_ena       : STD_LOGIC := '0';               --enables internal scl to output
  SIGNAL sda_int       : STD_LOGIC := '1';               --internal sda
  SIGNAL sda_ena_n     : STD_LOGIC;                      --enables internal sda to output
  SIGNAL devid_rw      : STD_LOGIC_VECTOR(7 DOWNTO 0);   --latched in device id and read/write
  signal regaddr		  : std_logic_vector(7 downto 0); 
  SIGNAL data_tx       : STD_LOGIC_VECTOR(7 DOWNTO 0);   --latched in data to write to slave
  SIGNAL data_rx       : STD_LOGIC_VECTOR(7 DOWNTO 0);   --data received from slave
  SIGNAL bit_cnt       : INTEGER RANGE 0 TO 7 := 7;      --tracks bit number in transaction
  SIGNAL stretch       : STD_LOGIC := '0';               --identifies if slave is stretching scl
  signal datacnt       : std_logic := '0'; 
  --signal count_unsigned       : unsigned(10 downto 0); 
BEGIN

  --generate the timing for the bus clock (scl_clk) and the data clock (data_clk)
  PROCESS(clk, reset)
    VARIABLE count  :  INTEGER RANGE 0 TO divider*4;  --timing for clock generation
  BEGIN
	 --count_unsigned <= to_unsigned(count, 11);
    IF(reset = '1') THEN                --reset asserted
      stretch <= '0';
      count := 0;
    ELSIF(rising_edge(clk)) THEN
      data_clk_prev <= data_clk;          --store previous value of data clock
      IF(count = divider*4-1) THEN        --end of timing cycle
        count := 0;                       --reset timer
      ELSIF(stretch = '0') THEN           --clock stretching from slave not detected
        count := count + 1;               --continue clock generation timing
      END IF;
      CASE count IS
        WHEN 0 TO divider-1 =>            --first 1/4 cycle of clocking
          scl_clk <= '0';
          data_clk <= '0';
        WHEN divider TO divider*2-1 =>    --second 1/4 cycle of clocking
          scl_clk <= '0';
          data_clk <= '1';
        WHEN divider*2 TO divider*3-1 =>  --third 1/4 cycle of clocking
          scl_clk <= '1';                 --release scl
          IF(scl = '0') THEN              --detect if slave is stretching clock
            stretch <= '1';
          ELSE
            stretch <= '0';
          END IF;
          data_clk <= '1';
        WHEN OTHERS =>                    --last 1/4 cycle of clocking
          scl_clk <= '1';
          data_clk <= '0';
      END CASE;
    END IF;
  END PROCESS;

--state machine and writing to sda during scl low (data_clk rising edge)
  PROCESS(clk, reset, datacnt)
  BEGIN
    IF(reset = '1') THEN                 --reset asserted
      state <= ready;                      --return to initial state
      busy <= '1';                         --indicate not available
      scl_ena <= '0';                      --sets scl high impedance
      sda_int <= '1';                      --sets sda high impedance
      ack_error <= '0';                    --clear acknowledge error flag
      bit_cnt <= 7;                        --restarts data bit counter
      data_rd <= "00000000";               --clear data read port
		datacnt <= '0';
    ELSIF(rising_edge(clk)) THEN
      IF(data_clk = '1' AND data_clk_prev = '0') THEN  --data clock rising edge
        CASE state IS
          WHEN ready =>                      --idle state
            IF(ena = '1') THEN               --transaction requested
              busy <= '1';                   --flag busy
              devid_rw <= devid & rw;          --collect requested slave device id and command
              data_tx <= data_wr;            --collect requested data to write
				  regaddr <= addr; 
              state <= start;                --go to start bit
            ELSE                             --remain idle
              busy <= '0';                   --unflag busy
              state <= ready;                --remain idle
            END IF;
				datacnt <= '0';                  --reset data counter 
          WHEN start =>                      --start bit of transaction
            busy <= '1';                     --resume busy if continuous mode
            sda_int <= devid_rw(bit_cnt);     --set first device id bit to bus
            state <= command;                --go to command
          WHEN command =>                    --device id and command byte of transaction
            IF(bit_cnt = 0) THEN             --command transmit finished
              sda_int <= '1';                --release sda for slave acknowledge
              bit_cnt <= 7;                  --reset bit counter for "byte" states
              state <= slv_ack1;             --go to slave acknowledge (command)
            ELSE                             --next clock cycle of command state
              bit_cnt <= bit_cnt - 1;        --keep track of transaction bits
              sda_int <= devid_rw(bit_cnt-1); --write device id/command bit to bus
              state <= command;              --continue with command
            END IF;
          WHEN slv_ack1 =>                   --slave acknowledge bit (command)
            IF(devid_rw(0) = '0') THEN        --write command
              sda_int <= regaddr(bit_cnt);   --write first bit of data
              state <= wr;                   --go to write byte
            ELSE                             --read command
              sda_int <= '1';                --release sda from incoming data
              state <= rd;                   --go to read byte
            END IF;
          WHEN wr =>                         --write byte of transaction
            busy <= '1';                     --resume busy if continuous mode
            IF(bit_cnt = 0) THEN             --write byte transmit finished
              sda_int <= '1';                --release sda for slave acknowledge
              bit_cnt <= 7;                  --reset bit counter for "byte" states
              state <= slv_ack2;             --go to slave acknowledge (write)
            ELSE                             --next clock cycle of write state
              bit_cnt <= bit_cnt - 1;        --keep track of transaction bits
				  if datacnt = '0' then 
						sda_int <= regaddr(bit_cnt-1); --write next bit to bus
				  else 
						sda_int <= data_tx(bit_cnt-1); --write next bit to bus
				  end if; 
              state <= wr;                   --continue writing
            END IF;
          WHEN rd =>                         --read byte of transaction
            busy <= '1';                     --resume busy if continuous mode
            IF(bit_cnt = 0) THEN             --read byte receive finished
              IF(ena = '1' AND devid_rw = devid & rw) THEN  --continuing with another read at same device id
                sda_int <= '0';              --acknowledge the byte has been received
              ELSE                           --stopping or continuing with a write
                sda_int <= '1';              --send a no-acknowledge (before stop or repeated start)
              END IF;
              bit_cnt <= 7;                  --reset bit counter for "byte" states
              data_rd <= data_rx;            --output received data
              state <= mstr_ack;             --go to master acknowledge
            ELSE                             --next clock cycle of read state
              bit_cnt <= bit_cnt - 1;        --keep track of transaction bits
              state <= rd;                   --continue reading
            END IF;
          WHEN slv_ack2 =>                   --slave acknowledge bit (write)
            IF(datacnt = '0') THEN               --continue transaction
              busy <= '1';                   --continue is accepted
              --devid_rw <= devid & rw;          --collect requested slave device id and command
              sda_int <= data_tx(bit_cnt); --write first bit of data
              state <= wr;                 --go to write byte
				  datacnt <= '1';
            ELSE                             --complete transaction
              state <= stop;                 --go to stop bit
            END IF;
          WHEN mstr_ack =>                   --master acknowledge bit after a read
            IF(ena = '1') THEN               --continue transaction
              busy <= '0';                   --continue is accepted and data received is available on bus
              devid_rw <= devid & rw;          --collect requested slave device id and command
              data_tx <= data_wr;            --collect requested data to write
              IF(devid_rw = devid & rw) THEN   --continue transaction with another read
                sda_int <= '1';              --release sda from incoming data
                state <= rd;                 --go to read byte
              ELSE                           --continue transaction with a write or new slave
                state <= start;              --repeated start
              END IF;    
            ELSE                             --complete transaction
              state <= stop;                 --go to stop bit
            END IF;
          WHEN stop =>                       --stop bit of transaction
            busy <= '0';                     --unflag busy
            state <= ready;                  --go to idle state
        END CASE;    
      ELSIF(data_clk = '0' AND data_clk_prev = '1') THEN  --data clock falling edge
        CASE state IS
          WHEN start =>                  
            IF(scl_ena = '0') THEN                  --starting new transaction
              scl_ena <= '1';                       --enable scl output
              ack_error <= '0';                     --reset acknowledge error output
            END IF;
          WHEN slv_ack1 =>                          --receiving slave acknowledge (command)
            IF(sda /= '0' OR ack_error = '1') THEN  --no-acknowledge or previous no-acknowledge
              ack_error <= '1';                     --set error output if no-acknowledge
            END IF;
          WHEN rd =>                                --receiving slave data
            data_rx(bit_cnt) <= sda;                --receive current slave data bit
          WHEN slv_ack2 =>                          --receiving slave acknowledge (write)
            IF(sda /= '0' OR ack_error = '1') THEN  --no-acknowledge or previous no-acknowledge
              ack_error <= '1';                     --set error output if no-acknowledge
            END IF;
          WHEN stop =>
            scl_ena <= '0';                         --disable scl
          WHEN OTHERS =>
            NULL;
        END CASE;
      END IF;
    END IF;
  END PROCESS;  

  --set sda output
  WITH state SELECT
    sda_ena_n <= data_clk_prev WHEN start,     --generate start condition
                 NOT data_clk_prev WHEN stop,  --generate stop condition
                 sda_int WHEN OTHERS;          --set to internal sda signal    
      
  --set scl and sda outputs
  scl <= '0' WHEN (scl_ena = '1' AND scl_clk = '0') ELSE 'Z';
  sda <= '0' WHEN sda_ena_n = '0' ELSE 'Z';

end Behavioral;

