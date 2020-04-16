# Open Ephys ONI API Implementation
The host-side APIs allow high-performance software interfaces and the creation
of high level language bindings. API implementations are compliant with the
[Open Neuro Interface specification](https://github.com/jonnew/ONI).

- [`liboni`](liboni) is an ANSI-C implementation of the [Open Ephys++ API
  Specificaiton](liboni#api-spec). It contains functions for configuring and
  streaming data to and from hardware.
- [`cpponi`](cpponi) C++14 bindings for `liboni`.
- [`clroni`](clroni) CLR/.NET bindings for `liboni`.

Minimal example host programs for each of these libraries can be found in the
\*-test folder within each library directory. If you are interested in writing
a binding or integrating and existing API with your software, please  get in
touch.
