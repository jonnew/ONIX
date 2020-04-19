# Riffa driver for Windows
## Using a test-mode driver
The driver is not digitally signed by microsoft, but in a developer testing phase. To being able to use it follow the following steps:

1. Install the testing certificate
	1. Find the OpenEphysTestDriver.cer file and double click it
	2. Select "Install Certificate"
	3. It is recommended to select "local user" certificate storage
	4. For ease of management, it is recommended to manually select a certificate storage and chose "Personal"
2. Put Windows in testing mode
	1. Open the start menu and write "cmd" to find the command prompt application
	2. Right click it and chose "Run as Administrator"
	3. execute the command "bcdedit /set testsigning on"
	4. Reboot the computer
	
The computer will keep the test state until you run "bcdedit /set testsigning off" in an administrator command propmt again and reboot the computer

## Installing the driver
**IMPORTANT:** The riffa driver conflicts with xillybus. Completely uninstall the xillybus driver before installing riffa.
### Prebuilt
This folder includes a prebuilt driver for 64-bit Windows 10. After the computer is set in testing mode, select the preferred version, right click the riffa.inf file and select "install".

### From code
To build from code you will need
- [Visual Studio 2019](https://visualstudio.microsoft.com/es/vs/)
- [Windows Driver Kit (WDK)](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)

Follow the instructions on the WDK webpage and then open the riffa.sln Visual Studio Solution file.

Driver test signing might not be enabled. To do so open the project properties and under "Driver Signing/General" for "Test Certificate" select the provided "OpenEphysTestDriver.pfx" file. If VS asks for a password enter "open".

	