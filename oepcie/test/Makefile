CC=cc
CFLAGS=-g -O0 -Wall
LDFLAGS=-loepcie

#liboepcie
LIBOEPCIE_SOURCES=$(wildcard ../liboepcie/*.c) 
LIBOEPCIE_OBJS=$(LIBOEPCIE_SOURCES:../liboepcie/%.c=%.o)

all: firmware host 

liboepcie.a: $(LIBOEPCIE_SOURCES)
	@echo Found liboepcie source files: $(LIBOEPCIE_SOURCES)
	$(CC) -c $(CFLAGS) $?
	@echo Generating liboepcie archive
	ar -rcs liboepcie.a $(LIBOEPCIE_OBJS)

firmware: firmware128.c testfunc.c liboepcie.a
	@echo Making $@
	#@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? -lm -lpthread $(LDFLAGS) -o $@

host: host128.c liboepcie.a
	@echo Making $@
	#@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

clean:
	rm -rf firmware host liboepcie.a $(LIBOEPCIE_OBJS)

.PHONY: liboepcie clean
