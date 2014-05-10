CC = /usr/bin/gcc
CFLAGS = -g -Wall

all: cdecl hex2dec dec2hex

hex2dec: hex2dec.c
	${CC} ${CFLAGS} hex2dec.c -o hex2dec -lm

dec2hex: dec2hex.c
	${CC} ${CFLAGS} dec2hex.c -o dec2hex -lm

cdecl: cdecl.c
	${CC} ${CFLAGS} cdecl.c -o cdecl 

cdecl_test: cdecl
	./cdecl "char (*(*x[3])())[5];"

hex2dec_test: hex2dec dec2hex
	./hex2dec 0xFFFFFF | ./dec2hex
