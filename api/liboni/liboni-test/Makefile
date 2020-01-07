# Examples:
# make 			// Makes all test programs
# make debug 	// Makes all test programs with debug symbols
# make profile 	// Makes all test programs and links in the gperftool's profiler

CC=cc
CFLAGS=-O3 -Wall
LDFLAGS=-loni -ldl

.PHONY: all
all: bin/firm_xilly bin/host_xilly bin/cobs_test ## Build the firmware emulator, basic host, and helper functions

.PHONY: debug
debug: CFLAGS += -DDEBUG -g3 ## Build with debug symbols
debug: all

.PHONY: profile
profile: LDFLAGS += -lprofiler  ## Link in the perftools profiler
profile: all

bin/cobs_test: cobs_test.c ## Make COBS test program
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? -o $@

bin/firm_xilly: firm_xilly.c testfunc.c ## Make xillybus-based firmware emulator
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? -lm -lpthread $(LDFLAGS) -o $@

bin/host_xilly: host_xilly.c ## Make simple host acquisition software targeting xillybus-based firmware
	@echo Making $@
	@[ -d bin ] || mkdir bin
	$(CC) $(CFLAGS) $? -lm -lpthread $(LDFLAGS) -o $@

.PHONY: clean
clean: ## Clean build artifacts
	rm -r ./bin

.PHONY: help
help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
