PROF ?= -fprofile-arcs -ftest-coverage -g -O0
CFLAGS ?= -W -Wall -pedantic -O3 $(PROF) $(CFLAGS_EXTRA) -std=c99
CXXFLAGS ?= -W -Wall -pedantic -O3 $(PROF) $(CFLAGS_EXTRA)
CLFLAGS ?= /DWIN32_LEAN_AND_MEAN /MD /O2 /TC /W2 /WX

RD ?= docker run -v $(CURDIR):$(CURDIR) -w $(CURDIR)
GCC ?= $(RD) docker.cesanta.com/gcc

.PHONY: clean all vc98 vc2017 c c++

all: ci-test
ci-test: c c++ vc98 vc2017

c: clean
	$(GCC) cc unit_test.c -o unit_test $(CFLAGS) && $(GCC) ./unit_test
	$(GCC) gcov -a unit_test.c

c++: clean
	$(GCC) g++ unit_test.c -o unit_test $(CXXFLAGS) && $(GCC) ./unit_test
	$(GCC) gcov -a unit_test.c

vc98 vc2017:
	$(RD) docker.cesanta.com/$@ wine cl unit_test.c $(CLFLAGS) /Fe$@.exe
	$(RD) docker.cesanta.com/$@ wine $@.exe 

coverity: clean
	rm -rf cov-int
	nice cov-build --dir cov-int $(MAKE) c GCC= COVERITY=1

clean:
	rm -rf *.gc* *.dSYM unit_test *.exe *.obj _CL_*
