# Make sure you add ./oepcie.conf to /etc/ld.so.conf.d so that DLLImport can find
# liboepcie.so
test: ../clroepcie/oepcie.dll
	cp ../clroepcie/oepcie.dll ./
	mcs -r:../clroepcie/oepcie.dll Host.cs

.PHONY: clean
clean:
	$(RM) oepcie.dll
	$(RM) Host.exe
