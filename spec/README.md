# Open Ephys++ Specification [WIP]

The goals of this project are bigger than simply producing headstage and
acquisition hardware for our own needs. We have found that proper headstage
serialization and host to PC communication are some of the trickiest parts of
the development process. Therefore developers often resort to brittle solutions
that meet their needs but are hard to inter-operate (e.g. NI DAQs).
Alternatively, they may turn to ready-made options with poor performance (e.g.
Opal Kelly development boards). Therefore, these interfaces become the limiting
hardware for both terms of integration (e.g. using disparate hardware in the
same rig)  as well as closed-loop performance (e.g. loop delay).

To address this, we have created a set of __general purpose__ communication
protocols, device drivers, and programming interfaces to support __arbitrary
mixtures of hardware__. These designs are formally described by the [Open Ephys++
Specificaiton](spec.pdf), which was developed with lots of community input.

The hope is that this acquisition system will get the boring part out of the
way: it serves as a high performance, open-source platform upon which cool
neuroscience tools can by built. We also hope that the use of a common
protocols will allow easy integration of disparate hardware to develop rich
feedback control loops for neuroscience projects. If you are interested in
developing against this specification (e.g. for your miniscope, headstage,
position tracker, 2P microscope, etc), we would love to [hear from
you](https://gitter.im/open-ephys-pcie/Lobby) and talk about the best way to
proceed. If you prefer a private conversation, please contact us via [Open
Ephys](http://www.open-ephys.org/contact/).

## Contents
The Open [Open Ephys++ Specificaiton](spec.pdf) formally defines the
requirements of the following components:

1. Headstage to host serialization protocol specification
1. Host to PC communication protocol specification
1. Host device driver specification [WIP]
1. Host programming interface specification

In addition to these specifications, we have created the following flagship
implementations which form data acquisition system at the base of our [next
generation headstages](../headstage-64). These implementations are modular --
you can use them form your project!  No need to reinvent the wheel:


- Firmware implementations based on (1,2).
    - TODO

- Device driver implementation based on (3)
    - TODO
- API implementations based upon (4):
    - [liboepcie](../api/liboepcie) is an ANSI-C open-ephys++ API implementation.
    It contains functions for configuring and stream data to and from hardware.
    - [cppoepcie](../api/cppoepcie) C++14 bindings for liboepcie.
    - [clroepcie](../api/clroepcie) CLR/.NET bindings for liboepcie.

If you think these implementations suck, feel free to use the spec to develop
for yourself. Your product will be automatically compatible with Open Ephys
tools!

## Feedback
We know we did not do a perfect job with this specification. [Please tell us
why!](https://gitter.im/open-ephys-pcie/Lobby) -- it will help this project be
generally useful for the neuroscience research.

## License
[MIT](https://en.wikipedia.org/wiki/MIT_License)
