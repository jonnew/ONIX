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
	@[ -d $(DESTDIR)$(PREFIX)/lib/oepcie ] || mkdir -p $(DESTDIR)$(PREFIX)/lib/oepcie
	@[ -d $(DESTDIR)$(PREFIX)/include/oepcie ] || mkdir -p $(DESTDIR)$(PREFIX)/include/oepcie
	cp $(SNAME) $(DESTDIR)$(PREFIX)/lib/oepcie/$(SNAME)
	cp $(DNAME) $(DESTDIR)$(PREFIX)/lib/oepcie/$(DNAME)
	cp $(HDR) $(DESTDIR)$(PREFIX)/include/oepcie
	@[ -d $(DESTDIR)$(PREFIX)/lib/oepcie/$(DNAMELN) ] || $(RM) $(DESTDIR)$(PREFIX)/lib/oepcie/$(DNAMELN)	
	ln -s $(DESTDIR)$(PREFIX)/lib/oepcie/$(DNAME) $(DESTDIR)$(PREFIX)/lib/oepcie/$(DNAMELN)

.PHONY: uninstall
uninstall:
	$(RM) -r $(DESTDIR)$(PREFIX)/lib/oepcie
	$(RM) -r $(DESTDIR)$(PREFIX)/include/oepcie

$(SNAME): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(DNAME): LDFLAGS += -shared
$(DNAME): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(OBJ)
	$(RM) $(SNAME) $(DNAME)
