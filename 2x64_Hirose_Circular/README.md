# 128 channel headstage module for rat tetrode drives
A very low profile digital headstage for amplifying, filtering, and digitizing microwire voltage data from a microdrive implant. Up to 256 wires (64 tetrodes) can be aquired.

## Features
- Circular form factor conforms with most _tetrode drive_ assemblies
  - Low profile, stacked connectorization using 0.4mm pitch [Hirose DF40](http://www.digikey.com/product-search/en?FV=ffec4097) mezzanine headers
- SPI interface options for:
  - 128 channels using a single LVDS bus (32 tetrodes)
  - 256 channels using a double LVDS bus (64 tetrodes)
- Integrated electrode plating and impedance testing

## TODO

- [x] Analog connections terminating at the inputs to an RHD chip do not need to feed through to the next board. However, digital connections _always_ should feed through because they need to make it to the headstage interface
 - Is there a way to prevent stubs in the digital feed troughs that will go the wrong way?
- [x] Does it make a difference if the header or the receptacle is next to the RHD, specifically in regard to stack height. i.e. is there going to be enough room for the potted chip?
 - Selected a connector that has various options for stack height, up to 3mm which will be more than enough
- [x] Make sure the circuit has _ample test-points_, especially for probing GND, VDD, REF, ELEC_TEST, AUX_OUT.
 - I did my best, but was not able to get the ELEC_TEST pinned out due to routing issues. However, these signals have only a standard CMOS analog switch between their wire entry point and the RHD chip, so I'm not sure test points onboard are super critical.
- [x] Address Reid's concern about routing high impedance ELEC_TEST lines near digital signals
- [x] Add three axis accelerometer to one of the RHD's aux analog inputs
    - Ask for Reid's opinion on the possibility of this chip introducing HF noise into the RHD chip sitting right below it.
    - Reid did not think it would be an issue
- [ ] Stack height concerns
    - The ADX is sitting right over an RHD. This leads to an effective stack height of their combined thinkness. Solution: only populate the to ADX since thats all you need anyway.
    - The 74HC4053 is a pretty thick chip (standard TSOP). I got 2 mm stack height headers. Will it fit? Its 1.75 mm thick max, so it should fit (barely). Also, need to take into account that the bottom headstage is rotated 90 deg. relative to the top. It looks like nothing will move underneath the 74HC4053 so long as the bottom ADX is not populated, but I need to test this on some cheapo boards before making the real ones!
    - Should get stack height of potted die from Reid just to be safe.

## Hardware and Documentation Licensing
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" property="dct:title">128 Channel Circular Hirose Headstage Module</span> by <a xmlns:cc="http://creativecommons.org/ns#" href="https://github.com/jonnew/cyclops" property="cc:attributionName" rel="cc:attributionURL">Jonathan P. Newman</a> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.<br />Based on a work at <a xmlns:dct="http://purl.org/dc/terms/" href="https://github.com/jonnew/cyclops" rel="dct:source">https://github.mit.edu/jpnewman/headstage</a>.


