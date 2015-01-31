## TODO
Jonnew's TODOs for the 128 channel headstage.

- [x] Analog connections terminating at the inputs to an RHD chip do not need to feedthrough to the next board. However, digitcal connections _always_ should feed through because they need to make it to the headstage interface
 - Is there a way to prevent stubs in the digital feedthroughs that will go the wrong way?
-[x] Does it make a difference if the header or the recepticle is next to the RHD, specifically in regard to stack height. i.e. is there going to be enough room for the potted chip?
 - Selected a connector that has various options for stack height, up to 3mm which will be more than enough
- [x] Make sure the circuit has _ample test-points_, espeically for probing GND, VDD, REF, ELEC_TEST, AUX_OUT.
 - I did my best. was not able to get the ELEC_TEST pinned out due to routing issues. However, these signals have only a standard cmos analog switch between their wire entry point and the RHD chip, so I'm not sure test points onboard are super critical.
