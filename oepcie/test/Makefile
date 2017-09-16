CC=cc
CFLAGS=-g -O0 -Wall
LDFLAGS=-loepcie

#liboepcie
LIBOEPCIE_SOURCES=$(wildcard ../liboepcie/*.c) 
LIBOEPCIE_OBJS=$(LIBOEPCIE_SOURCES:../liboepcie/%.c=%.o)

all: bin/firmware bin/host bin/signal_test #bin/writer bin/reader

liboepcie.a: $(LIBOEPCIE_SOURCES)
	@echo Found liboepcie source files: $(LIBOEPCIE_SOURCES)
	$(CC) -c $(CFLAGS) $?
	@echo Generating liboepcie archive
	ar -rcs liboepcie.a $(LIBOEPCIE_OBJS)

bin/firmware: firmware128.c testfunc.c liboepcie.a
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? -lm -lpthread $(LDFLAGS) -o $@

bin/host: host128.c liboepcie.a
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

bin/signal_test: signal_test.c testfunc.c liboepcie.a
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

clean:
	rm -rf bin liboepcie.a $(LIBOEPCIE_OBJS)

.PHONY: liboepcie clean
