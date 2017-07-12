----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    15:23:34 07/10/2017 
-- Design Name: 
-- Module Name:    input - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity input is
port(
   --master global clock and reset 
	clk_in : in std_logic;  --input clock form oscillator 
	reset  : in std_logic;  --master system reset 
	
	--I2C interface 
	sda 	 : inout std_logic; 
	scl    : out std_logic; 
	gpio_in   : in std_logic_vector(2 downto 0); --gpio input (used for fast triggering of stimulations)
	
	--control signals to slave blocks 
	--Intans 
	intan_clk : out std_logic; 
	intan_regaddr : out unsigned(4 downto 0); 
	intan_regval  : out unsigned(7 downto 0); 
	intan_regconf : out std_logic; 
	intan_regack  : in std_logic; 
	intan_adcconf : out std_logic; 
	
	--GPIO
	gpio_sel : out std_logic_vector(2 downto 0); 
	
	--LED 1s
	led_conf_clk : out std_logic; --configuration clock for the led interface 
	led1_trig    : out std_logic; --led trigger pulse 
	led1_conf    : out std_logic; --led configuration pulse to start a new conf cycle 
	led1_intensity : out std_logic_vector(15 downto 0); --led intensity value 

	pot_value    : out std_logic_vector(7 downto 0); --pot value
	pot_conf     : out std_logic; --pot configuration pulse 

	led2_trig    : out std_logic; --led trigger pulse 
	led2_conf    : out std_logic; 
	led2_intensity : out std_logic_vector(15 downto 0)

	--IMU
	--to be completed 

	);
end input;

architecture Behavioral of input is

begin


end Behavioral;

