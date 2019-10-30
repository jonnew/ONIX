# clroepcie 
CLR/.NET bindings for [`liboepcie`](../liboepcie/README.md).

## Build

### Visual Studio (Windows)
1. Open the `clrpepcie.sln` solution in visual studio. 
2. "Running" the solution will compile the library and test program, and then
   run the test program

__Notes (aka please help...)__

- I removed the Any CPU build option because I don't understand how to use it
  properly. Instead, I build two times for each x64 and x86
- Each build type has a post build event in which the liboepcie.dll (of
  appropriate architecture) is copied from the Externals folder to the target
  directory so that it will be included in the nuget package.

__Creating the nuget package__
```
nuget pack clroepcie.csproj -properties Configuration=Release
```

### Mono
[Mono](https://github.com/mono/mono) is an open source .NET implementation.
`mcs` is the mono C# compiler.

```
$ cd clroepcie
$ make
```

## Test Programs
The [clroepcie-test](clroepcie-test) directory contains minimal working
programs that use this library

1. `Host.exe` : Basic data acquisition loop. Communicate with
   `liboepoe-test/firmware` or actual hardware.

This will be automatically built when the visual studio solution is built. It
can also be built using mono via

```
$ cd clroepcie-test
$ make
```

## License
[MIT](https://en.wikipedia.org/wiki/MIT_License)

## Issues
- Pausing data does not make sense if it does not occur on natural data boundaries. If im going to have an OE_RUNNING option, then it just needs to keep reading frames (at the software level) and dumping them with as little overhead as possible.
- Reseting causes infinite read() if there is not data being produced on the read channel. This could be solved by putting info device into heartbeat, or just giving it a heartbeat all the time.
- This means that once started after reset, the info device should not respect a pause! According to first comment, nothing should respect pause at the hardware level.
- Reset does not flush fifos properly. They need to be closed() and re-opened().
