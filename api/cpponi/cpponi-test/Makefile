# Examples:
# make 			// Makes all test programs
# make debug 	// Makes all test programs with debug symbols
# make profile 	// Makes all test programs and links in the gperftool's profiler

CC=c++

CFLAGS=-O3 -std=c++2a -stdlib=libc++ -Wall
LDFLAGS=-loni

.PHONY: all
all: host_xilly

.PHONY: debug
debug: CFLAGS += -DDEBUG -g3
debug: all

.PHONY: profile
profile: LDFLAGS += -lprofiler
profile: all

host_xilly: host_xilly.cpp
	@echo Making $@
	$(CC) $(CFLAGS) $? -lm -lpthread -ldl $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm host_xilly
