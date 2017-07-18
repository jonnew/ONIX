all : test_1 test_2 test_3

test_1 : test_1.c ../liboepcie/oepcie.c
	clang test_1.c ../liboepcie/oepcie.c -g -o test_1

test_2 : test_2.c ../liboepcie/oepcie.c
	clang -g test_2.c ../liboepcie/oepcie.c -o test_2

test_3 : test_3.c ../liboepcie/oepcie.c
	clang -g test_3.c ../liboepcie/oepcie.c -o test_3

clean  : 
	rm test_1 test_2 test_3