# liboepcie CLR/.NET binding

## Build notes
- I removed the Any CPU build option because I don't understand how to use it properly. Instead, I build two times for each x64 and x86
- Each build type has a post build event in which the liboepcie.dll (of appropriate architecture) is copied from the Externals folder to the target directory so that it will be included in the nuget package (I think).

## Creating the nuget package
Use the following command:
```
nuget pack clroepcie.csproj -properties Configuration=Release
```