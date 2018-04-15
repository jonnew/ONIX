--This is the input port definitions (work in progress)
--It defines input/outputs of the input block.

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

entity input is
	port (
		--master global clock and reset
		clk_in : in std_logic; --input clock form oscillator
		reset  : in std_logic; --master system reset
 
		--I2C interface
		sda     : inout std_logic;
		scl     : out std_logic;
		gpio_in : in std_logic_vector(2 downto 0); --gpio input (used for fast triggering of stimulations)
 
		--control signals to slave blocks
		--Intans
		intan_clk     : out std_logic; --master clock to intan chips (used for generating spi signals)
		intan_regaddr : out unsigned(7 downto 0); --intan register addresses
		intan_regval  : out unsigned(7 downto 0); --intan register values 
		intan_regconf : out std_logic; --pulse to signal a configuration 
		intan_regack  : in std_logic;  --pulse means a register configuration is successful 
		intan_adcconf : out std_logic; --pulse to start adc configuration 
 
		--GPIO
		gpio_sel : out std_logic_vector(2 downto 0); --selection to either use GPIO to trigger LED or use it for other purposes
 
		--LED 1s
		led_conf_clk   : out std_logic; --configuration clock for the led interface
		led1_trig      : out std_logic; --led trigger pulse
		led1_conf      : out std_logic; --led configuration pulse to start a new conf cycle
		led1_intensity : out std_logic_vector(15 downto 0); --led intensity value

		pot_value      : out std_logic_vector(7 downto 0); --pot value
		pot_conf       : out std_logic; --pot configuration pulse

		led2_trig      : out std_logic; --led trigger pulse
		led2_conf      : out std_logic;
		led2_intensity : out std_logic_vector(15 downto 0)

		--IMU
		--to be completed

	);
end input;

architecture Behavioral of input is

begin
end Behavioral;