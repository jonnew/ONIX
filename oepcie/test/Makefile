CC=cc
CFLAGS=-g -O0 -Wall
LDFLAGS=
#SOURCES=../liboepcie/oepcie.c ../liboepcie/oeutil.c
#OBJECTS=$(SOURCES:.c=.o)

#.c.o:
#	$(CC) -c $(CFLAGS) $< -o $@

#liboepcie
LIBOEPCIE_SOURCES=$(wildcard ../liboepcie/*.c) #../liboepcie/oeutil.c
LIBOEPCIE_OBJS=$(LIBOEPCIE_SOURCES:../liboepcie/%.c=%.o)

all: bin/firmware

liboepcie: $(LIBOEPCIE_SOURCES)
	echo $(LIBOEPCIE_SOURCES)
	$(CC) -c $(CFLAGS) $^ -c #-o $(@:../liboepcie/%=%)
	ar -r -c -s  liboepcie.a $?

bin/firmware: firmware128.c liboepcie.a
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $^ -lm $(LDFLAGS) -o $@

#bin/host128: host128.c $(OBJECTS)
#	@[ -d bin ] || mkdir bin
#	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
#
#bin/writer: writer.c $(OBJECTS)
#	@[ -d bin ] || mkdir bin
#	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
#
#bin/reader: reader.c $(OBJECTS)
#	@[ -d bin ] || mkdir bin
#	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
#
clean:
	echo "rm -rf bin liboepcie.a $(LIBOEPCIE_OBJS)"

.PHONY: liboepcie clean
#.PHONY: bin/firmware128 bin/host128 .c.o clean bin/reader bin/writer

#CC=cc
#CFLAGS=-g -O0 -Wall
#LDFLAGS=-lm
#
##liboepcie
#LIBOEPCIE_SOURCES=$(wildcard ../../liboepcie/*.c) #../liboepcie/oeutil.c
#LIBOEPCIE_OBJS=$(LIBOEPCIE_SOURCES:../liboepcie/%.c=%.o)

#all: bin/firmware bin/host
#
#libobjs: $(LIBOEPCIE_SOURCES)
#	echo $(LIBOEPCIE_SOURCES)
#	$(CC) -c $(CFLAGS) $^ -c #-o $(@:../liboepcie/%=%)
#
#liboepcie: $(LIBOEPCIE_OBJS) libobjs
#	ar r liboepcie $?

#bin/firmware: firmware128.c liboepcie
#	@[ -d bin ] || mkdir bin
#	$(CC) $(CFLAGS) $< -L liboepcie -lm -o $@
#
#bin/host: host128.c liboepcie
#	@[ -d bin ] || mkdir bin
#	$(CC) $(CFLAGS) $< -L liboepcie -o $@

#bin/host: host128.c $(LIBOEPCIE_OBJS)
#	@[ -d bin ] || mkdir bin
#	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
#
#bin/writer: writer.c $(LIBOEPCIE_OBJS)
#	@[ -d bin ] || mkdir bin
#	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
#
#bin/reader: reader.c $(LIBOEPCIE_OBJS)
#	@[ -d bin ] || mkdir bin
#	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@


#clean:
#	rm -rf bin $(LIBOEPCIE_OBJS) liboepcie
#
#.PHONY: libobjs clean
