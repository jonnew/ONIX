oepcie.dll: oepcie.cs oedevices.cs
	mcs -unsafe -t:library oepcie.cs oedevices.cs Context.cs Device.cs Frame.cs OEException.cs

.PHONY: clean
clean:
	$(RM) oepcie.dll
