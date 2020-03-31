# Make sure you add ./oni.conf to /etc/ld.so.conf.d so that DLLImport can find
# liboepcie.so
test: ../clroni/oni.dll
	cp ../clroni/oni.dll ./
	mcs -r:../clroni/oni.dll Host.cs

.PHONY: clean
clean:
	$(RM) oni.dll
	$(RM) Host.exe
