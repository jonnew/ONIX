CC=c++

CFLAGS=-O3 -std=c++14 -stdlib=libc++ -Wall
LDFLAGS=-loepcie #-lprofiler

host: host.cpp
	@echo Making $@
	$(CC) $(CFLAGS) $? -lm -lpthread $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm host
