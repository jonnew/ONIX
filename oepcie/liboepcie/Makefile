# Examples:
# make                          // Makes the library without debug symbols
# make install                  // Installs library at default PREFIX
# make install PREFIX=somedir   // Installs library in somedir
# make debug                    // Makes the library with debug symbols
# make ENDFLAG=OE_BIGEND        // Turn on hardware endianess conversion
SHELL   :=  /bin/bash
NAME    :=  liboepcie
SNAME   :=  $(NAME).a
DNAME   :=  $(NAME).so.1
DNAMELN :=  $(NAME).so
HDR     :=  $(wildcard *.h)
SRC     :=  $(wildcard *.c)
OBJ     :=  $(SRC:.c=.o)
CFLAGS  :=  -pedantic -Wall -W -Werror -fPIC -O3 $(ENDFLAG)
LDFLAGS :=  -L.
PREFIX  := /usr/local

# Turn wildcard list into comma separated list
SPACE :=
SPACE += # $SPACE is a SPACE
COMMA := ,
COMMA-SEPARATE = $(subst ${SPACE},${COMMA},$(strip $1))

.PHONY: all
all: $(SNAME) $(DNAME)

.PHONY: debug
debug: CFLAGS += -DDEBUG -g3 
debug: all

.PHONY: install
install: $(SNAME) $(DNAME)
	@[ -d $(DESTDIR)$(PREFIX)/lib ] || mkdir -p $(DESTDIR)$(PREFIX)/lib
	@[ -d $(DESTDIR)$(PREFIX)/include ] || mkdir -p $(DESTDIR)$(PREFIX)/include
	cp $(SNAME) $(DESTDIR)$(PREFIX)/lib/$(SNAME)
	cp $(DNAME) $(DESTDIR)$(PREFIX)/lib/$(DNAME)
	cp $(HDR) $(DESTDIR)$(PREFIX)/include
	@[ -d $(DESTDIR)$(PREFIX)/lib/$(DNAMELN) ] || $(RM) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)
	ln -s $(DESTDIR)$(PREFIX)/lib/$(DNAME) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)

.PHONY: uninstall
uninstall:
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
clean:
	$(RM) $(OBJ)
	$(RM) $(SNAME) $(DNAME)
