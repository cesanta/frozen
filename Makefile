PROF ?= -fprofile-arcs -ftest-coverage
CFLAGS ?= -std=c99 -g -O0 -W -Wall -pedantic $(CFLAGS_EXTRA)
CXXFLAGS ?= -g -O0 -W -Wall -pedantic $(CFLAGS_EXTRA)
CLFLAGS ?= /DWIN32_LEAN_AND_MEAN /MD /O2 /TC /W2 /WX

RD ?= docker run --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR)
GCC ?= $(RD) docker.cesanta.com/gcc

.PHONY: all asan c c++ clean vc98 vc2017

all: ci-test
ci-test: asan c c++ vc98 vc2017

c: clean
	$(GCC) cc unit_test.c -o unit_test $(CFLAGS) $(PROF) && $(GCC) ./unit_test
	$(GCC) gcov -a unit_test.c

c++: clean
	$(GCC) c++ unit_test.c -o unit_test $(CXXFLAGS) $(PROF) && $(GCC) ./unit_test
	$(GCC) gcov -a unit_test.c

vc98 vc2017:
	$(RD) docker.cesanta.com/$@ wine cl unit_test.c $(CLFLAGS) /Fe$@.exe
	$(RD) docker.cesanta.com/$@ wine $@.exe 

asan:
	$(RD) -e ASAN_OPTIONS=symbolize=1 docker.cesanta.com/clang \
	  clang unit_test.c -o unit_test $(CFLAGS) -fsanitize=address && \
	$(RD) docker.cesanta.com/clang ./unit_test

coverity: clean
	rm -rf cov-int
	nice cov-build --dir cov-int $(MAKE) c GCC= COVERITY=1

clean:
	rm -rf *.gc* *.dSYM unit_test *.exe *.obj _CL_*
