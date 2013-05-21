headstage
=========

Description
----------------
The Open Ephys headstage uses an [Intan amplifier chip](http://www.intantech.com) to filter, amplify, and multiplex 32 channels of neural data. An analog-to-digital converter inside the chip digitizes each signal with 16-bit resolution. Instead of one wire per channel, as is required for analog headstages, our headstage can send 32 channels over a cable with only 12 wires. And the signal is purely digital, so there are no artifacts from tether movement.

View on [Open Ephys](http://open-ephys.com/headstage/).

If you're interested in building your own headstage, we strongly recommend getting in touch with us via the Open Ephys [contact](http://open-ephys.com/contact) page.

Details
----------
Traditionally, headstages serve as a buffer between high-impedance electrodes recording signals from the brain and low-impedance wires that carry signals to a rack-mounted filter bank and amplifier array. They are sometimes referred to as "preamplifiers," because they boost current without amplifying voltage. But if the filters and amplifiers can be miniaturized and combined into a single chip, preamplification is no longer necessary. This is what the Intan chips make possible, which is why we were so excited to incorporate them into our headstage design.

There are many advantages to using the Intan chips in place of a traditional analog signal chain. For one, it reduces the cost of a multichannel data acquisition system by at least an order of magnitude, making it more affordable to purchase a system with a high channel count, or many smaller systems at once. It also reduces the footprint of the system, since the amplification steps that once required a large box can now fit inside an 8x8 mm chip. And it allows us to digitize neural signals before they leave the headstage—eliminating the common cable artifacts seen when sending analog signals over long tethers.

One disadvantage of this approach is that it reduces flexibility in choosing an analog reference. Our headstages contain a dedicated reference channel, but once you've chosen your reference electrode for a given implant, it can't be changed. External amplifiers make it possible to manually select a new reference channel at any point during a recording.

Specifications
---------------------
- 32 channels
- upper cutoff frequency adjustable from 100 Hz to 20 kHz
- lower cutoff frequency adjustable from 0.1 Hz to 500 Hz
- 2.4 µV rms noise
- 16-bit resolution
- red power indicator LED
- 3-axis accelerometer
- 3.3V power supply

About the different versions
-------------------------------------
- **standard:** 22 x 13 mm
- **miniature:** 17.5 x 12 mm, with the same specs as the "standard" version
- **samtec:** compatible with 50 mil Samtec connectors

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
