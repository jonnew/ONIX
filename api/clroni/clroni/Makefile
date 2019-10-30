onipcie.dll: oni.cs onidevices.cs
	mcs -unsafe -t:library oni.cs onidevices.cs Context.cs Device.cs Frame.cs ONIException.cs

.PHONY: clean
clean:
	$(RM) oni.dll
