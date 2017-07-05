## KC705 Firmware

Verilog code for the driving the acquisition system using a [KC705 evaluation board](https://www.xilinx.com/products/boards-and-kits/ek-k7-kc705-g.html)

### File hierarchy

-Sources/Verilog: Verilog source files
-Sources/IP: IP template files
-Sources/Constraints: Constraints files
-Other: Other files that might be needed in the project. The extra search paths of the project includes this one.


### Notes
Note that intermediate build artifacts and folders should not end up in the source repo will be deleted when rebuilding the project.

Verilog and constraints files are added to the project without copying. This means that modifying them inside Vivado will modify the original files, useful for version control.
IP template files, however, will be copied inside the project. This is because Vivado will fail when trying to load an IP file updated by a newer version. If a change to an IP instance is needed, manual checks will be needed before overwriting.