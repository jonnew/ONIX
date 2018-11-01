# cppoepcie
C++14 bindings for [`liboepcie`](../liboepcie/README.md).

## Build
This is a header only library, so there is nothing to build. Just run
```
$ make install PREFIX=/path/to/install
```
to place headers in whatever path is specified by PREFIX. PREFIX defaults to
`/usr/lib/include`. You can uninstall (delete headers) via
```
$ make uninstall PREFIX=/path/to/uninstall
```

## Test Programs
The [cppoepcie-test](cppoepcie-test) directory contains minimal working
programs that use this library

1. `host` : Basic data acquisition loop. Communicate with `liboepoe-test/firmware` or actual
   hardware.

## Performance testing
1. Install google perftools:
```
$ sudo apt-get install google-perftools
```
2. Link test programs against the CPU profiler:
```
$ cd cppoepcie-test
$ make profile
```
5. Run the C `firmware` program to serve fake data. Provide a numerical argument
   specifying the number of fake frames to produce. It will tell you how long
   it takes `host` to sink all these frames. This is host processing time +
   UNIX pipe read/write.
```
$ cd ../liboepcie/liboepcie-test/bin
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

## License
[MIT](https://en.wikipedia.org/wiki/MIT_License)
