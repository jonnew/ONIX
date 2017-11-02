NAME    :=  liboepcie
SNAME   :=  $(NAME).a
DNAME   :=  $(NAME).so.1
DNAMELN :=  $(NAME).so
HDR     :=  $(wildcard *.h)
SRC     :=  $(wildcard *.c)
OBJ     :=  $(SRC:.c=.o)
CFLAGS  :=  -pedantic -Wall -W -Werror -g -g3 -fPIC -O3
LDFLAGS :=  -L.
PREFIX  := /usr/local

.PHONY: all
all: $(SNAME) $(DNAME)

.PHONY: install
install: $(SNAME) $(DNAME)
	@[ -d $(DESTDIR)$(PREFIX)/lib ] || mkdir -p $(DESTDIR)$(PREFIX)/lib
	@[ -d $(DESTDIR)$(PREFIX)/include ] || mkdir -p $(DESTDIR)$(PREFIX)/include
	cp $(SNAME) $(DESTDIR)$(PREFIX)/lib/$(SNAME)
	cp $(DNAME) $(DESTDIR)$(PREFIX)/lib/$(DNAME)
	cp $(HDR) $(DESTDIR)$(PREFIX)/include
	@[ -d $(DESTDIR)$(PREFIX)/lib/$(DNAMELN) ] || $(RM) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)
	ln -s $(DESTDIR)$(PREFIX)/lib/$(DNAME) $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)

#.PHONY: uninstall
#uninstall:
#	$(RM) -r $(DESTDIR)$(PREFIX)/lib/$(SNAME)
#	$(RM) -r $(DESTDIR)$(PREFIX)/lib/$(DNAME)
#	$(RM) -r $(DESTDIR)$(PREFIX)/lib/$(DNAMELN)
#	$(RM) -r $(DESTDIR)$(PREFIX)/include/$(HDR)

$(SNAME): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(DNAME): LDFLAGS += -shared
$(DNAME): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(OBJ)
	$(RM) $(SNAME) $(DNAME)
