# Examples:
# make 			// Makes all test programs
# make debug 	// Makes all test programs with debug symbols
# make profile 	// Makes all test programs and links in the gperftool's profiler

CC=c++

CFLAGS=-O3 -std=c++14 -stdlib=libc++ -Wall
LDFLAGS=-loepcie

.PHONY: all
all: host

.PHONY: debug
debug: CFLAGS += -DDEBUG -g3
debug: all

.PHONY: profile
profile: LDFLAGS += -lprofiler
profile: all

host: host.cpp
	@echo Making $@
	$(CC) $(CFLAGS) $? -lm -lpthread $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm host
