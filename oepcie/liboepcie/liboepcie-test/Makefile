CC=cc
CFLAGS=-g -g3 -O3 -Wall 
LDFLAGS=-loepcie #-lprofiler

all: bin/firmware bin/host bin/cobs_test

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
