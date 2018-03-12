CC=cc
CFLAGS=-g -O0 -Wall 
LDFLAGS=-loepcie

#liboepcie
#LIBOEPCIE_SOURCES=$(wildcard ../liboepcie/*.c) 
#LIBOEPCIE_OBJS=$(LIBOEPCIE_SOURCES:../liboepcie/%.c=%.o)

all: bin/firmware bin/host  bin/cobs_test

#liboepcie.a: $(LIBOEPCIE_SOURCES)
#	@echo Found liboepcie source files: $(LIBOEPCIE_SOURCES)
#	$(CC) -c $(CFLAGS) $?
#	@echo Generating liboepcie archive
#	ar -rcs liboepcie.a $(LIBOEPCIE_OBJS)

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
	rm -rf bin liboepcie.a $(LIBOEPCIE_OBJS)

.PHONY: liboepcie clean
