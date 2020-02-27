# "make help" prints help.
SHELL     :=  /bin/bash
NAME      :=  libonidriver_riffa
DNAME     :=  $(NAME).so.1
DNAMELN   :=  $(NAME).so
SRC       :=  onidriver_riffa.c
OBJ       :=  $(SRC:.c=.o)
CFLAGS    :=  -pedantic -Wall -W -Werror -fPIC -O3 $(DEFS)
LDFLAGS   :=  -L.
PREFIX    :=  /usr/local

# Turn wildcard list into comma separated list
SPACE :=
SPACE += # $SPACE is a SPACE
COMMA := ,
COMMA-SEPARATE = $(subst ${SPACE},${COMMA},$(strip $1))

.PHONY: all
all: $(DNAME) ## Make release version of onidriver_riffa.

.PHONY: debug
debug: CFLAGS += -DDEBUG -g3 ## Make driver with debug symbols.
debug: all

.PHONY: install 
install: $(SNAME) $(DNAME) ## Install driver. Defaults to make install PREFIX=/usr/local.
	@[ -d $(DESTDIR)$(PREFIX)/lib ] || mkdir -p $(DESTDIR)$(PREFIX)/lib
	@[ -d $(DESTDIR)$(PREFIX)/include ] || mkdir -p $(DESTDIR)$(PREFIX)/include
	cp $(DNAME) $(DESTDIR)$(PREFIX)/lib/$(DNAME)
	@[ -d $(DESTDIR)$(PREFIX)/lib/$(DNAMELN) ] || $(RM) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)
	ln -s $(DESTDIR)$(PREFIX)/lib/$(DNAME) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)
	ldconfig
	ldconfig -p | grep libonidriver

.PHONY: uninstall
uninstall: ## Remove driver from installation directory.
	$(RM) $(DESTDIR)$(PREFIX)/lib/$(DNAME)
	$(RM) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)

$(DNAME): LDFLAGS += -shared
$(DNAME): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean: ## Remove local build objects
	$(RM) $(OBJ)
	$(RM) $(DNAME)

.PHONY: help
help:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
