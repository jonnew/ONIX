oepcie.dll: oepcie.cs
	mcs -unsafe -t:library oepcie.cs Error.cs

# Make sure you add ./oepcie.conf to /etc/ld.so.conf.d so that DLLImport can find
# liboepcie.so 
test: oepcie.dll
	mcs -r:oepcie.dll Test.cs
	./Test.exe

.PHONY: clean
clean:
	$(RM) oepcie.dll
	$(RM) Test.exe
