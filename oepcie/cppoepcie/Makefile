NAME    :=  liboepcie
PREFIX  := /usr/local

.PHONY: install
install: $(SNAME) $(DNAME)
	@[ -d $(DESTDIR)$(PREFIX)/include/ ] || mkdir -p $(DESTDIR)$(PREFIX)/include/oepcie
	cp oepcie.hpp $(DESTDIR)$(PREFIX)/include/

.PHONY: uninstall
uninstall:
	$(RM) -r $(DESTDIR)$(PREFIX)/include/oepcie.hpp

