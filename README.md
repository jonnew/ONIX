headstage
=========

Description
----------------
The Open Ephys headstage uses an [Intan amplifier chip](http://www.intantech.com) to filter, amplify, and multiplex 32 channels of neural data. An onboard analog-to-digital converter digitizes each signal with 16-bit resolution. Instead of one wire per channel, as is required for analog headstages, our headstage can send 32 channels over as few as 7 wires. And the signal is purely digital, so there are no artifacts from tether movement.

Cost of raw materials: $457.37

Time to build: 1.5 hours

View on [Open Ephys](http://open-ephys.com/headstage/).

If you're interested in building your own headstage, we strongly recommend getting in touch with us via the Open Ephys [contact](http://open-ephys.com/contact) page.

Details
----------
Traditionally, headstages serve as a buffer between high-impedance electrodes recording signals from the brain and low-impedance wires that carry signals to a rack-mounted filter bank and amplifier array. They are sometimes referred to as "preamplifiers," because they boost current without amplifying voltage. But if the filters and amplifiers can be miniaturized and combined into a single chip, preamplification is no longer necessary. This is what the Intan chips make possible, which is why we were so excited to incorporate them into our headstage design.

There are many advantages to using the Intan chips in place of a traditional analog signal chain. For one, it reduces the cost of a multichannel data acquisition system by at least an order of magnitude, making it more affordable to purchase a system with a high channel count, or many smaller systems at once. It also reduces the footprint of the system, since the amplification steps that once required a large box can now fit inside an 8x8 mm chip. And it allows us to digitize neural signals before they leave the headstage—eliminating the common cable artifacts seen when sending analog signals over long tethers.

One disadvantage of this approach is that it reduces flexibility in choosing an analog reference. Our headstages contain a dedicated reference channel, but once you've chosen your reference electrode for a given implant, it can't be changed. External amplifiers make it possible to manually select a new reference channel at any point during a recording.

Current specifications
-----------------------------
- 32 channels
- hardware filters between 1 Hz and 10 kHz
- +/-5 mV input range
- 16-bit resolution
- high-density Molex connector on the animal end
- HDMI type D ("micro") connector on the tether end
- red power indicator LED
- 3.3V power supply
- 2.5V onboard voltage regulator and voltage reference

Features that may be added in the future
-------------------------------------------------------
- I2C accelerometer
- Programmable filter settings
- User-selectable reference

Known issues
------------------
- In the current version of the headstage, the board and schematic are NOT consistent. This will be updated in future revisions. For now, please disregard any airwires, disconnected traces, or DRC errors that stem from Eagle detecting inconsistencies between the two files.
- Gold-immersion finish makes the white solder mask look slighly pinkish; let's switch to black in the future

File types
-------------
- .ai = Adobe Illustrator files; contain images of hardware
- .brd = EAGLE board files; describe the physical layout of the printed circuit board
- .sch = EAGLE schematic files; describe the electrical connections of the printed circuit board
- .cam = EAGLE export files; contain instructions for translating between the .brd file and Gerber files
- BOM.txt = Bill of Materials; contains part numbers for all components (from DigiKey unless otherwise specified)
- .md = Markdown files; most likely a README file; can be viewed with any text edtior
- "gerber" files (.top, .bsk, .oln, etc.) = contain machine-readable instructions for creating the printed circuit board; these are sent to a fab house (such as Sunstone Circuits) for PCB production
- .SLDPRT files = SolidWorks part files; contain CAD models of 3D components
- .STL files = stereolithography files; can be sent to a rapid prototyping service (such as Shapeways) to create 3D objects
- .eps file = encapsulated postscript files; describe the shape of laser-cut parts (Ponoko only). Can be edited in Adobe Illustrator.

DISCLAIMER: We don't recommend using any of the tools from Open Ephys for actual experiments until they've been tested more thoroughly. If you'd like to know which tests we've run or plan to run, please get in touch with us via the Open Ephys [contact](http://open-ephys.com/contact) page.
