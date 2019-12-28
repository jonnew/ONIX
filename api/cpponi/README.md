# cpponi
C++17 & C++2a bindings for [`liboni`](../liboni/README.md).

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
When compiling programs using this library, you can use `-std=c++2a` to expose
the `frame_t.data` method:

```c++
template <typename raw_t>
std::span<const raw_t> data(size_t dev_idx)
```

Otherwise you must rely on `begin()` and `end()` to use frame data.

## Test Programs
The [cpponi-test](cpponi-test) directory contains minimal working
programs that use this library

1. `host_xilly` : Basic data acquisition loop. Communicate with
   `liboni/liboni-test/firm_xilly` or actual hardware.

## Performance testing
1. Install google perftools:
```
$ sudo apt-get install google-perftools
```
2. Link test programs against the CPU profiler:
```
$ cd cpponi-test
$ make profile
```
5. Run the C `firmware` program to serve fake data. Provide a numerical argument
   specifying the number of fake frames to produce. It will tell you how long
   it takes `host` to sink all these frames. This is host processing time +
   UNIX pipe read/write.
```
$ cd ../liboni/liboni-test/bin
$ ./firm_xilly 10e6
```
4. Run the `host_xilly` program while dumping profile info:
```
$ env CPUPROFILE=/tmp/host.prof ./host_xilly /tmp/cmd_32 /tmp/signal_8 /tmp/read_32 /tmp/write_32
```
5. Examine output
```
$ google-pprof ./host_xilly /tmp/host.prof
```

## License
[MIT](https://en.wikipedia.org/wiki/MIT_License)
