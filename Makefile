CBASICFLAGS = -O0 -fno-inline -g -ggdb -Wall -Wextra -fsanitize=address,undefined -Werror
CVALGRINDFLAGS = -O0 -fno-inline -g -ggdb -Wall -Wextra -Werror
CFLAGS = $(CBASICFLAGS) -isystem $(GTEST_DIR)/include
CDEBUGFLAGS = $(CFLAGS) -DDEBUG=1

GTEST_DIR = $(HOME)/gitsrc/googletest
GTEST_HEADERS = $(GTEST_DIR)/googletest/include
GTESTLIBPATH=$(GTEST_DIR)/build/lib
GTESTLIBS = $(GTESTLIBPATH)/libgtest.a $(GTESTLIBPATH)/libgtest_main.a
LDBASICFLAGS= -g -fsanitize=address,undefined -lpthread
LDVALGRINDFLAGS= -g
LDFLAGS= $(LDBASICFLAGS) -L$(GTESTLIBPATH)

CC = /usr/bin/gcc
CPPCC = /usr/bin/g++

# Each subdirectory must supply rules for building sources it contributes
%.o: %.cc
	$(CPPCC) -isystem $(GTEST_HEADERS) $(CFLAGS) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

all: cdecl hex2dec dec2hex cpumask

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

cpumask: cpumask.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o cpumask cpumask.c -lm

# The test crashes ASAN.
cpumask_testsuite.o: cpumask_testsuite.cc
	$(CPPCC) -isystem $(GTEST_HEADERS) $(CVALGRINDFLAGS) -fsanitize=undefined -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

cpumask_test: cpumask_testsuite.o cpumask.c
	$(CPPCC) $(CVALGRINDFLAGS) $(LDVALGRINDFLAGS) -fsanitize=undefined -Wall -o cpumask_test cpumask_testsuite.o $(GTESTLIBS)

clean:
	/bin/rm -rf *.o *~ hex2dec dec2hex cdecl cpumask cpumask_test
