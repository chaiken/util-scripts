CBASICFLAGS = -O0 -fno-inline -g -ggdb -Wall -Wextra -fsanitize=address,undefined -Werror
CVALGRINDFLAGS = -O0 -fno-inline -g -ggdb -Wall -Wextra -Werror
CFLAGS = $(CBASICFLAGS) -isystem $(GTEST_DIR)/include
CDEBUGFLAGS = $(CFLAGS) -DDEBUG=1

CPPFLAGS= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS)
CPPFLAGS += -isystem $(GTEST_DIR)/include

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

CPPLAGS-NOTEST= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=address,undefined
LDFLAGS-NOTEST= -ggdb -g -fsanitize=address

# “–coverage” is a synonym for-fprofile-arcs, -ftest-coverage(compiling) and
# -lgcov(linking).
COVERAGE_EXTRA_FLAGS = --coverage
CXXFLAGS-NOSANITIZE= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -I$(GTEST_HEADERS) -isystem $(GTEST_DIR)/include
LDFLAGS-NOSANITIZE= -ggdb -g -L$(GTESTLIBPATH)
%lib_test-coverage: CPPFLAGS = $(CXXFLAGS-NOSANITIZE) $(COVERAGE_EXTRA_FLAGS)
%lib_test-coverage: LDFLAGS = $(LDFLAGS-NOSANITIZE)
%lib_test-coverage:  %lib_test.cc %lib.cc $(GTESTHEADERS)
	$(CPPCC) $(CPPFLAGS)  $(LDFLAGS) $^ $(GTESTLIBS) -o $@
	run_lcov.sh $@

# https://clang.llvm.org/extra/clang-tidy/
# https://stackoverflow.com/questions/47583057/configure-clang-check-for-c-standard-libraries
# Without "-x c++", .h files are considered as C.
# Without header_filter, gtest.h is analyzed.
CLANG_TIDY_BINARY=/usr/bin/clang-tidy
CLANG_TIDY_OPTIONS=--warnings-as-errors --header_filter=.*
CLANG_TIDY_CLANG_OPTIONS=-std=c++17 -x c++ -I ~/gitsrc/googletest/googletest/include/
CLANG_TIDY_CHECKS=bugprone,core,cplusplus,cppcoreguidelines,deadcode,modernize,performance,readability,security,unix,apiModeling.StdCLibraryFunctions,apiModeling.google.GTest

# Has matching lib.cc file.
%_lib_test-clangtidy: %_lib_test.cc %_lib.cc %.h
	$(CLANG_TIDY_BINARY) $(CLANG_TIDY_OPTIONS) -checks=$(CLANG_TIDY_CHECKS) $^ -- $(CLANG_TIDY_CLANG_OPTIONS)

# http://www.valgrind.org/docs/manual/quick-start.html#quick-start.prepare
# Compile your program with -g . . . Using -O0 is also a good idea,
# cc1plus: error: ‘-fsanitize=address’ and ‘-fsanitize=kernel-address’ are incompatible with ‘-fsanitize=thread’
# https://www.gnu.org/software/make/manual/make.html#Pattern_002dspecific
# Without the flags redefinition, setting the object files as prerequisites of
# %_lib_test-valgrind would trigger compilation with the rules above, which has
# the wrong flags.   Valgrind does not work with the sanitizers.
# "As with target-specific variable values, multiple pattern values create a
# pattern-specific variable value for each pattern individually."
%_test-valgrind template_%_lib_test-valgrind : CPPFLAGS-VALGRIND = $(CXXFLAGS-NOSANITIZE) -O0
%_test-valgrind template_%_lib_test-valgrind :  LDFLAGS-VALGRIND = $(LDFLAGS-NOSANITIZE)
%_test-valgrind: %_test.cc %.cc $(GTESTHEADERS)
	$(CPPCC) $(CPPFLAGS-VALGRIND) $(LDFLAGS-VALGRIND) $^ $(GTESTLIBS) -o $@

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

classify_process_affinity_lib_test: classify_process_affinity_lib.cc classify_process_affinity.h classify_process_affinity_lib_test.cc
	$(CPPCC) $(CPPFLAGS) $(LDFLAGS)  classify_process_affinity_lib.cc classify_process_affinity_lib_test.cc  $(GTESTLIBS) -o $@

classify_process_affinity: classify_process_affinity.cc classify_process_affinity_lib.cc classify_process_affinity.h
	$(CPPCC) $(CPPFLAGS-NOTEST) $(LDFLAGS-NOTEST)  classify_process_affinity_lib.cc classify_process_affinity.cc -o $@

%_lib_test-clangtidy: %_lib_test.cc %_lib.cc %.h
	$(CLANG_TIDY_BINARY) $(CLANG_TIDY_OPTIONS) -checks=$(CLANG_TIDY_CHECKS) $^ -- $(CLANG_TIDY_CLANG_OPTIONS)

clean:
	/bin/rm -rf *.o *.d *~ hex2dec dec2hex cdecl watch_file watch_one_file cpumask cpumask_gtest cpumask_ctest classify_process_affinity_lib_test *coverage *gcda *gcno *info *css *html *valgrind *png util-scripts
