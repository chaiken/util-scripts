CBASICFLAGS = -O0 -fno-inline -g -ggdb -Wall -Wextra -fsanitize=address,undefined -Werror
CVALGRINDFLAGS = -O0 -fno-inline -g -ggdb -Wall -Wextra -Werror
CFLAGS = $(CBASICFLAGS) -isystem $(GTEST_DIR)/include
CDEBUGFLAGS = $(CFLAGS) -DDEBUG=1

GTEST_DIR = $(HOME)/gitsrc/googletest
GTEST_HEADERS = $(GTEST_DIR)/googletest/include
GTESTLIBPATH=$(GTEST_DIR)/build/lib
GTESTLIBS = $(GTESTLIBPATH)/libgtest.a $(GTESTLIBPATH)/libgtest_main.a
LDBASICFLAGS= -g -fsanitize=address,undefined
LDVALGRINDFLAGS= -g
LDFLAGS= $(LDBASICFLAGS) -L$(GTESTLIBPATH)

CATCH_DIR = $(HOME)/gitsrc/Catch2
# "If you are using the two file distribution instead, remember to replace the
# included header with catch_amalgamated.hpp."
CATCH_HEADERS = $(CATCH_DIR)/extras/
CATCHLIBPATH = $(CATCH_DIR)/build/src
# The ordering of the libraries matters, as only libCatch2Main.a needs the symbols in libCatch2.a.
CATCHLIBS = $(CATCHLIBPATH)/libCatch2Main.a  $(CATCHLIBPATH)/libCatch2.a 
LDCATCHFLAGS =  $(LDBASICFLAGS) -L$(CATCHLIBPATH)

CC = /usr/bin/gcc
CPPCC = /usr/bin/g++

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

# How to crash ASAN:
# Comment out the stanza below.
# Change
#	$(CVALGRINDFLAGS) $(LDVALGRINDFLAGS) -fsanitize=undefined
# to
# 	$(CFLAGS) $(LDFLAGS)
# make cpumask_test
# ./cpumask_test
cpumask_testsuite.o: cpumask_testsuite.cc
	$(CPPCC) -isystem $(GTEST_HEADERS) $(CVALGRINDFLAGS) -fsanitize=undefined -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

cpumask_gtest: cpumask_testsuite.o cpumask.c
	$(CPPCC) $(CVALGRINDFLAGS) $(LDVALGRINDFLAGS) -fsanitize=undefined -Wall -o cpumask_gtest cpumask_testsuite.o $(GTESTLIBS)

cpumask-valgrind: cpumask.c
	$(CC) $(CVALGRINDFLAGS) $(LDVALGRINDFLAGS) -o cpumask-valgrind cpumask.c -lm

cpumask_ctest.o: cpumask_ctest.cc
	$(CPPCC) -isystem $(CATCH_HEADERS) $(CBASICFLAGS) -fsanitize=undefined -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

cpumask_ctest: cpumask_ctest.o cpumask.c
	$(CPPCC) -isystem $(CATCH_HEADERS) $(CBASICFLAGS) $(LDCATCHFLAGS) -o cpumask_ctest cpumask_ctest.o $(CATCHLIBS)

clean:
	/bin/rm -rf *.o *.d *~ hex2dec dec2hex cdecl watch_file watch_one_file cpumask cpumask_gtest cpumask_ctest 
