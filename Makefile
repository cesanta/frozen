PROF ?= -fprofile-arcs -ftest-coverage -g
CFLAGS ?= -std=c99 -g -O0 -W -Wall -pedantic $(CFLAGS_EXTRA)
CXXFLAGS ?= -g -O0 -W -Wall -pedantic $(CFLAGS_EXTRA)
CLFLAGS ?= /DWIN32_LEAN_AND_MEAN /MD /O2 /TC /W2 /WX

RD ?= docker run --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR)
DOCKER_ROOT ?= docker.io/mgos
GCC ?= $(RD) $(DOCKER_ROOT)/gcc

.PHONY: all asan c c++ clean vc98 vc2017

all: ci-test

ci-test: asan c c++ minimal vc98 vc2017

minimal:
	$(MAKE) asan c c++ CFLAGS_EXTRA=-DJSON_MINIMAL=1

c: clean
	$(GCC) cc unit_test.c -o unit_test $(CFLAGS) $(PROF) && $(GCC) ./unit_test
	$(GCC) gcov -a unit_test.c

c++: clean
	$(GCC) c++ unit_test.c -o unit_test $(CXXFLAGS) $(PROF) && $(GCC) ./unit_test
	$(GCC) gcov -a unit_test.c

vc98 vc2017:
	$(RD) $(DOCKER_ROOT)/$@ wine cl unit_test.c $(CLFLAGS) /Fe$@.exe
	$(RD) $(DOCKER_ROOT)/$@ wine $@.exe

asan:
	$(RD) -e ASAN_OPTIONS=symbolize=1 $(DOCKER_ROOT)/clang \
	  clang unit_test.c -o unit_test $(CFLAGS) -fsanitize=address && \
	$(RD) $(DOCKER_ROOT)/clang ./unit_test

coverity: clean
	rm -rf cov-int
	nice cov-build --dir cov-int $(MAKE) c GCC= COVERITY=1

clean:
	rm -rf *.gc* *.dSYM unit_test *.exe *.obj _CL_*
