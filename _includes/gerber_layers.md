PCB gerber files are located in the [gerber](gerber) folder. Files within this
folder are defined as follows:

 - \*.GKO = board outline 
 - \*.GTS = top solder mask 
 - \*.GBS = bottom solder mask
 - \*.GTO = top silk screen 
 - \*.GBO = bottom silk screen 
 - \*.GTL = top copper
 - \*.GnL = inner layer n copper 
 - \*.GBL = bottom copper 
 - \*.XLN[.xxyy] = drill hits and sizes. Files specifying blind or buried also
   specify start (xx) and end (yy) layers as additional extension.
 - \*.gvp = [gerbv](http://gerbv.geda-project.org/) project file.

If available, solder stencil gerber files are located in the [stencil](stencil)
folder. Files within this folder are defined as follows:

 - \*.CST = top-side stencil
 - \*.SST = bottom-side stencil
