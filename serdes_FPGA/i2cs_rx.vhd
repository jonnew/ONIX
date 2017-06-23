------------------------------------------------------------------------------
-- i2c slave receiver
------------------------------------------------------------------------------
-- Intended target: Xilinx CoolRunner-II CPLD  (XC2C64A)
-- Development tools: XILINX ISE 7.1i webpack
-- Author:  DMITRY PETROV
-- Notes:
-- Date:     11-09-2005
-- Revision: 1.0
------------------------------------------------------------------------------
-- This code implements i2c slave which is able to receive a data byte.
 
-- i2c message has 3 parts:
-- <Device Address> 22h
-- <Sub Address> 00h
-- <Data byte> XX 
-- If Device and Sub Addresses are matched the data byte will be accepted.

-- Because of SCL line used as clock for i2c state machine, slow SCL changes 
-- will make noise and invalid data reception. 

-- To avoid noise of slow SCL - usualy used an external CLOCK, for 
-- clocking all modules but it will take some amount of CPLD's macrocells.

-- Another way is to use an SCHMITT TRIGGER on SCL and SDA. 
-- Forexample XILINX CoolRunner-II CPLD, has SCHMITT TRIGGER on it's I/O.
-- By default this function is deactivated, PLEASE ACTIVATE IT !

-- If your PLD have no SCHMITT TRIGGER function, you may use solution wich 
-- require 2 resistors and aditional output pin. 
-- Here's an old Xilinx app note about it: 
-- http://www.xilinx.com/xcell/xl19/xl19-34.pdf 
-- ===========================================================================
-- DISCLAIMER: This code is FREEWARE which is provided on an “as is” basis, 
-- YOU MAY USE IT ON YOUR OWN RISK, WITHOUT ANY WARRANTY. 
-- ===========================================================================d

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

------------------------------------------------------------------------------
entity i2cs_rx is
	generic(
		WR       : std_logic:='0';
		DADDR		: std_logic_vector(6 downto 0); --:= "0010001";		   -- 11h (22h) device address
		ADDR		: std_logic_vector(7 downto 0)  --:= "00000000"		   -- 00h	    sub address		
	);
	port(
		RST		: in std_logic;
		SCL		: in std_logic;
		SDA		: inout std_logic;
		DOUT 		: out std_logic_vector(7 downto 0)			   -- Recepted over i2c data byte
	);
	--SCHMITT TRIGGER activation (folowing 3 strings should be uncommented)
	--attribute SCHMITT_TRIGGER: string; 
	--attribute SCHMITT_TRIGGER of SCL: signal is "true"; 
	--attribute SCHMITT_TRIGGER of SDA: signal is "true";
end i2cs_rx;

------------------------------------------------------------------------------
architecture Behavioral of i2cs_rx is
	signal DOUT_S: std_logic_vector(7 downto 0);
	signal SDA_IN, START, START_RST, STOP, ACTIVE, ACK	: std_logic;
	signal SHIFTREG	: std_logic_vector(8 downto 0);
	signal STATE : std_logic_vector(1 downto 0);		-- 00 - iddle state	
																			-- 01 - DADDR  compare
																			-- 10 - ADDR compare
																			-- 11 - DATA read
begin

------------------------------------------------------------------------------
-- start condition detection, method 1 ( good noise tolerance )

   process (SDA_IN, START_RST)
   begin
      if (START_RST = '1') then
         START <= '0';    
      elsif (SDA_IN'event and SDA_IN = '0') then
         START <= scl;    
      end if;
   end process;

   process (SCL, START, STOP)
   begin
		if (SCL'event and SCL = '0') then
         START_RST <= START;    
      end if;
   end process;

------------------------------------------------------------------------------
-- start condition detection, method 2 ( simple - but week against noise )
--process (RST, SCL, SDA_IN)
--begin
--	if RST = '0' or SCL = '0' then
--		START <= '0';
--	elsif SCL = '1' and SDA_IN = '0' and SDA_IN'event then
--		START <= '1';
--	end if;
--end process;

------------------------------------------------------------------------------
-- stop condition detection
process (RST, SCL, SDA_IN, START)
begin
	if RST = '0' or SCL = '0' or START='1' then
		STOP <= '0';
	elsif SCL = '1' and SDA_IN = '1' and SDA_IN'event then
		STOP <= '1';
	end if;
end process;

------------------------------------------------------------------------------
-- "active communication" signal 
process (RST, STOP, START)
begin
	if RST = '0' or STOP = '1' then	 --or (SHIFTREG="000000001" and ACK = '0' and SCL='1' and SCL'event) 
		ACTIVE <= '0';
	elsif START = '0' and START'event then
		ACTIVE <= '1';
	end if;
end process;

------------------------------------------------------------------------------
-- RX data shifter
process (RST, ACTIVE, ACK, SCL, SDA_IN)
begin 
if RST = '0' or ACTIVE = '0' then
	SHIFTREG <= "000000001";	
elsif SCL'event and SCL = '1' then
	if ACK = '1' then
		SHIFTREG <= "000000001";
	else
		SHIFTREG(8 downto 0) <= SHIFTREG(7 downto 0) & SDA_IN;
	end if;
end if;							  
end process;

------------------------------------------------------------------------------
-- I2C data read
process (RST, STATE, ACK, SHIFTREG)
begin
if RST = '0' then
	DOUT_S <= "00000000";
elsif STATE="11" and ACK='1' and ACK'event then 
	DOUT_S <= SHIFTREG(7 downto 0);
end if;
end process; 

------------------------------------------------------------------------------
-- ACK
process (RST, SCL, SHIFTREG, STATE, ACTIVE)
begin
if RST = '0' or ACTIVE = '0' then
	ACK <= '0';
	STATE <= "00";
elsif SCL='0' and SCL'event then 
	if SHIFTREG(8) = '1' and STATE/="11" then
		STATE <= STATE + 1;
		if ((STATE="00" and SHIFTREG(7 downto 0) = DADDR & WR) or (STATE="01" and SHIFTREG(7 downto 0) = ADDR) or STATE="10") then 
			ACK <= '1';
		else
			STATE <= "11";
		end if;
	else
	   ACK <= '0';
	end if;
end if;
end process;

------------------------------------------------------------------------------
-- ACK responce
SDA_IN <= SDA;
SDA <= '0' when ACK = '1' else 'Z';

------------------------------------------------------------------------------
DOUT(7 downto 0) <= DOUT_S(7 downto 0);

end Behavioral;
------------------------------------------------------------------------------