### KC705 Firmware specification (work in progress) 

There is a dedicated .vhd module to handle data communication for each of the three communication ports on the Xillybus:

## Signal channel (8-bit, asynchronous, read only)

# async_com_control.vhd 

This module controls the signal interface. It sends COBS encoded streams to the 8-bit width communication channel according to the host specification.

# cobs_encoder.vhd 

This is a COBS encoder written in VHDL. 

## Configuration channel (32-bit, synchronous, read and write)

# mem_conf_control.vhd 

This module interfaces with the configuration channel. It interprets the command send in by the host PC and reacts accordingly. 

## Data input channel (32-bit, asynchronous, read-only)

# hs_com_control.vhd 

This module handles the multi-sensor buffering and multiplexing to the xillybus 32bits data FIFO

