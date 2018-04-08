oepcie.dll: oepcie.cs
	mcs -unsafe -t:library oepcie.cs Context.cs DispoIntPtr.cs Frame.cs OEException.cs 

.PHONY: clean
clean:
	$(RM) oepcie.dll
