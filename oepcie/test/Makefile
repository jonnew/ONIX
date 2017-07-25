CC=gcc
IDIR=../liboepcie/
LDIR=../liboepcie/
CFLAGS="-g -I$(IDIR)"

all : test_1 #test_2 test_3

test_1 : test_1.c ../liboepcie/oepcie.c
	$(CC) test_1.c ../liboepcie/oepcie.c -g -o $@

#test_2 : test_2.c ../liboepcie/oepcie.c
#	$(CC) -g test_2.c ../liboepcie/oepcie.c -o $@
#
#test_3 : test_3.c ../liboepcie/oepcie.c
#	$(CC) -g test_3.c ../liboepcie/oepcie.c -o $@

.phony: clean

clean  : 
	rm test_1 test_2 test_3
