# Examples:
# make 			// Makes all test programs
# make debug 	// Makes all test programs with debug symbols
# make profile 	// Makes all test programs and links in the gperftool's profiler

CC=cc
CFLAGS=-O3 -Wall
LDFLAGS=-loepcie

.PHONY: all
all: bin/firmware bin/host bin/cobs_test

.PHONY: debug
debug: CFLAGS += -DDEBUG -g3
debug: all

.PHONY: profile
profile: LDFLAGS += -lprofiler
profile: all

bin/cobs_test: cobs_test.c
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? -o $@

bin/firmware: firmware.c testfunc.c
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? -lm -lpthread $(LDFLAGS) -o $@

bin/host: host.c
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? -lm -lpthread $(LDFLAGS) -o $@

clean:
	rm -r ./bin

.PHONY: clean
