# `liboepcie`
ANSI C implementation of the [Open Ephys++ API Specification](../spec.pdf).

## Build
For build options, look at the [Makefile](Makefile). To build and install:
```
$ make <options>
$ make install PREFIX=/path/to/install
```
to place headers in whatever path is specified by PREFIX. PREFIX defaults to
`/usr/lib/include`. You can uninstall (delete headers and libraries) via
```
$ make uninstall PREFIX=/path/to/uninstall
```

## Test Programs
The [liboepcie-test](liboepcie-test) directory contains minimal working programs that use this library.

1. `firmware` : Emulate hardware. Stream fake data over UNIX pipes (Linux only)
1. `host` : Basic data acquisition loop. Communicate with `firmware` or actual
   hardware (Linux and Windows).

## Performance testing
1. Install google perftools:
```
$ sudo apt-get install google-perftools
```
2. Link test programs against the CPU profiler:
```
$ cd liboepcie-test
$ make profile
```
5. Run the `firmware` program to serve fake data. Provide a numerical argument
   specifying the number of fake frames to produce. It will tell you how long
   it takes `host` to sink all these frames. This is host processing time +
   UNIX pipe read/write.
```
$ cd bin
$ ./firmware 10e6
```
4. Run the `host` program while dumping profile info:
```
$ env CPUPROFILE=/tmp/host.prof ./host /tmp/xillybus_cmd_mem_32 /tmp/xillybus_async_read_8 /tmp/xillybus_data_read_32
```
5. Examine output
```
$ pprof ./host /tmp/host.prof
```

## Memory testing
Run the `host` program with valgrind using full leak check
```
$ valgrind --leak-check=full ./host /tmp/xillybus_cmd_mem_32 /tmp/xillybus_async_read_8 /tmp/xillybus_data_read_32
```

## License
[MIT](https://en.wikipedia.org/wiki/MIT_License)
