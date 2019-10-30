# "make help" prints help.
SHELL   :=  /bin/bash
NAME    :=  liboni
SNAME   :=  $(NAME).a
DNAME   :=  $(NAME).so.1
DNAMELN :=  $(NAME).so
HDR     :=  $(wildcard *.h)
SRC     :=  $(wildcard *.c)
OBJ     :=  $(SRC:.c=.o)
CFLAGS  :=  -pedantic -Wall -W -Werror -fPIC -O3 $(DEFS)
LDFLAGS :=  -L.
PREFIX  := /usr/local

# Turn wildcard list into comma separated list
SPACE :=
SPACE += # $SPACE is a SPACE
COMMA := ,
COMMA-SEPARATE = $(subst ${SPACE},${COMMA},$(strip $1))

.PHONY: all
all: $(SNAME) $(DNAME) ## Make release version of liboni.

.PHONY: debug
debug: CFLAGS += -DDEBUG -g3 ## Make liboni with debug symbols.
debug: all

.PHONY: install 
install: $(SNAME) $(DNAME) ## Install liboni. Defaults to make install PREFIX=/usr/local.
	@[ -d $(DESTDIR)$(PREFIX)/lib ] || mkdir -p $(DESTDIR)$(PREFIX)/lib
	@[ -d $(DESTDIR)$(PREFIX)/include ] || mkdir -p $(DESTDIR)$(PREFIX)/include
	cp $(SNAME) $(DESTDIR)$(PREFIX)/lib/$(SNAME)
	cp $(DNAME) $(DESTDIR)$(PREFIX)/lib/$(DNAME)
	cp $(HDR) $(DESTDIR)$(PREFIX)/include
	@[ -d $(DESTDIR)$(PREFIX)/lib/$(DNAMELN) ] || $(RM) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)
	ln -s $(DESTDIR)$(PREFIX)/lib/$(DNAME) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)

.PHONY: uninstall
uninstall: ## Remove liboni from installation directory.
	$(RM) $(DESTDIR)$(PREFIX)/lib/$(SNAME)
	$(RM) $(DESTDIR)$(PREFIX)/lib/$(DNAME)
	$(RM) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)
	$(RM) $(DESTDIR)$(PREFIX)/include/{$(call COMMA-SEPARATE,${HDR})}

$(SNAME): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(DNAME): LDFLAGS += -shared
$(DNAME): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean: ## Remove local build objects
	$(RM) $(OBJ)
	$(RM) $(SNAME) $(DNAME)

.PHONY: help
help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
