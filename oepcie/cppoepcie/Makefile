PREFIX  := /usr/local/include/

.PHONY: install
install: 
	@[ -d $(DESTDIR)$(PREFIX) ] || mkdir -p $(DESTDIR)$(PREFIX)oepcie
	cp oepcie.hpp oedevices.hpp $(DESTDIR)$(PREFIX)

.PHONY: uninstall
uninstall:
	$(RM) -r $(DESTDIR)$(PREFIX)oepcie.hpp
	$(RM) -r $(DESTDIR)$(PREFIX)oedevices.hpp
