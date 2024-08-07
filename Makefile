CBASICFLAGS = -O0 -fno-inline -g -ggdb -Wall -Wextra -fsanitize=address,undefined -Werror
CVALGRINDFLAGS = -O0 -fno-inline -g -ggdb -Wall -Wextra -Werror
CFLAGS = $(CBASICFLAGS) -isystem $(GTEST_DIR)/include
CDEBUGFLAGS = $(CFLAGS) -DDEBUG=1

GTEST_DIR = $(HOME)/gitsrc/googletest
GTEST_HEADERS = $(GTEST_DIR)/googletest/include
GTESTLIBPATH=$(GTEST_DIR)/build/lib
GTESTLIBS = $(GTESTLIBPATH)/libgtest.a $(GTESTLIBPATH)/libgtest_main.a
GMOCK_HEADERS = $(GTEST_DIR)/googlemock/include
GMOCKLIBPATH=$(GTEST_DIR)/build/lib
GMOCKLIBS = $(GMOCKLIBPATH)/libgmock.a $(GTESTLIBPATH)/libgmock_main.a
LDBASICFLAGS= -g -fsanitize=address,undefined
LDVALGRINDFLAGS= -g
LDFLAGS= $(LDBASICFLAGS) -L$(GTESTLIBPATH) -L$(GMOCKLIBPATH)

CPPFLAGS= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS)
CPPFLAGS += -isystem $(GTEST_DIR)/include

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
CXXFLAGS-NOSANITIZE= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) -isystem $(GTEST_DIR)/include
LDFLAGS-NOSANITIZE= -ggdb -g -L$(GTESTLIBPATH)  -L$(GMOCKLIBPATH)
%lib_test-coverage: CPPFLAGS = $(CXXFLAGS-NOSANITIZE) $(COVERAGE_EXTRA_FLAGS)
%lib_test-coverage: LDFLAGS = $(LDFLAGS-NOSANITIZE)
%lib_test-coverage:  %lib_test.cc %lib.cc $(GTESTHEADERS)
	$(CPPCC) $(CPPFLAGS)  $(LDFLAGS) $^ $(GTESTLIBS) -o $@
	./run_lcov.sh $@

# https://clang.llvm.org/extra/clang-tidy/
# https://stackoverflow.com/questions/47583057/configure-clang-check-for-c-standard-libraries
# Without "-x c++", .h files are considered as C.
# Without header_filter, gtest.h is analyzed.
CLANG_TIDY_BINARY=/usr/bin/clang-tidy
CLANG_TIDY_OPTIONS=--warnings-as-errors --header_filter=.*
CLANG_TIDY_CLANG_OPTIONS=-std=c++17 -x c++ -I ~/gitsrc/googletest/googletest/include/
CLANG_TIDY_CHECKS=bugprone,core,cplusplus,cppcoreguidelines,deadcode,modernize,performance,readability,security,unix,apiModeling.StdCLibraryFunctions,apiModeling.google.GTest

# Has matching lib.cc file.
%_lib_test-clangtidy: %_lib_test.cc %_lib.cc %.hh
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

datasize: datasize.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o datasize datasize.c

hanoi: hanoi.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o hanoi hanoi.c

endian: endian.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o endian endian.c -lm

endian-cpp: endian.hh endian_lib.cc endian-cpp.cc
	$(CPPCC) $(CPPFLAGS) $(LDFLAGS) endian_lib.cc endian-cpp.cc -o endian-cpp

endian_lib_test: endian.hh endian_lib.cc endian_lib_test.cc
	$(CPPCC) $(CPPFLAGS) $(LDFLAGS) endian_lib.cc endian_lib_test.cc $(GTESTLIBS) -o $@

endian-cpp-valgrind: endian.hh endian_lib.cc endian-cpp.cc
	$(CPPCC) $(CVALGRINDFLAGS) $(LDVALGRINDFLAGS) endian_lib.cc endian-cpp.cc -o endian-cpp-valgrind -lm

cpumask: cpumask.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o cpumask cpumask.c -lm

linked_list: linked_list.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o linked_list linked_list.c

watch_file: watch_file.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o watch_file watch_file.c

watch_one_file: watch_one_file.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o watch_one_file watch_one_file.c

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

classify_process_affinity_lib_test: classify_process_affinity_lib.cc classify_process_affinity.hh classify_process_affinity_lib_test.cc
	$(CPPCC) $(CPPFLAGS) $(LDFLAGS)  classify_process_affinity_lib.cc classify_process_affinity_lib_test.cc  $(GTESTLIBS) -o $@

classify_process_affinity: classify_process_affinity.cc classify_process_affinity_lib.cc classify_process_affinity.hh
	$(CPPCC) $(CPPFLAGS-NOTEST) $(LDFLAGS-NOTEST)  classify_process_affinity_lib.cc classify_process_affinity.cc -o $@

timerlat_load_lib_test: timerlat_load_lib.cc timerlat_load.hh timerlat_load_lib_test.cc
	$(CPPCC) $(CPPFLAGS) $(LDFLAGS)  timerlat_load_lib.cc timerlat_load_lib_test.cc  $(GTESTLIBS) -o $@

timerlat_load: timerlat_load_lib.cc timerlat_load.hh timerlat_load.cc
	$(CPPCC) $(CPPFLAGS) $(LDFLAGS)  timerlat_load_lib.cc timerlat_load.cc -o $@

timerlat_pipe_load_lib_test: timerlat_pipe_load_lib.cc timerlat_pipe_load.hh timerlat_pipe_load_lib_test.cc
	$(CPPCC) $(CPPFLAGS) $(LDFLAGS)  timerlat_pipe_load_lib.cc timerlat_pipe_load_lib_test.cc  $(GTESTLIBS) -o $@

# https://stackoverflow.com/questions/73136532/where-is-the-data-race-in-this-simple-c-code
# UBSAN and TSAN together produce erroneous results.
timerlat_pipe_load_lib_test-tsan: timerlat_pipe_load_lib.cc timerlat_pipe_load.hh timerlat_pipe_load_lib_test.cc
	$(CPPCC) $(CXXFLAGS-NOSANITIZE) -fsanitize=thread $(LDFLAGS-NOSANITIZE) timerlat_pipe_load_lib.cc timerlat_pipe_load_lib_test.cc  $(GTESTLIBS) $(GMOCK_LIBS) -o $@

%_lib_test-clangtidy: %_lib_test.cc %_lib.cc %.hh
	$(CLANG_TIDY_BINARY) $(CLANG_TIDY_OPTIONS) -checks=$(CLANG_TIDY_CHECKS) $^ -- $(CLANG_TIDY_CLANG_OPTIONS)

BINARY_LIST = cdecl hex2dec dec2hex cpumask endian endian_lib_test watch_file watch_one_file endian-cpp endian_lib_test endian-cpp-valgrind cpumask cpumask_gtest cpumask-valgrind cpumask_ctest classify_process_affinity classify_process_affinity_lib_test timerlat_load_lib_test timerlat_load timerlat_pipe_load_lib_test timerlat_pipe_load_lib_test-tsan hanoi datasize linked_list

all:
	make $(BINARY_LIST)

clean:
	/bin/rm -rf $(BINARY_LIST) *.o *.d *~ watch_file watch_one_file cpumask cpumask_gtest cpumask_ctest classify_process_affinity_lib_test classify_process_affinity timerlat_pipe_load_lib_test timerlat_pipe_load_lib_test-tsan timerlat_load *coverage *gcda *gcno *info *css *html *valgrind *png *clangtidy
