# Open Ephys ONI API Implementation
The host-side APIs allow high-performance software interfaces and the creation
of high level language bindings. API implementations are compliant with the
[Open Neuro Interface specification](https://github.com/jonnew/ONI).

- [`liboepcie`](liboepcie) is an ANSI-C implementation of the [Open Ephys++ API
  Specificaiton](../spec/README.md). It contains functions for configuring and
  streaming data to and from hardware.
- [`cppoepcie`](cppoepcie) C++14 bindings for `liboepcie`.
- [`clroepcie`](clroepcie) CLR/.NET bindings for `liboepcie`.

Minimal example host programs for each of these libraries can be found in the
\*-test folder within each library directory. If you are interested in writing
a binding or integrating and existing API with your software, please  get in
touch.

## Using Xillybus with `liboepcie`
Instructions for using [Xillybus](http://xillybus.com/) as a communication
backend can be found [here](xillybus-backend.md).
