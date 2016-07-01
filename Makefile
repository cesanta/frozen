PROF = -fprofile-arcs -ftest-coverage -g -O0
CFLAGS = -W -Wall -pedantic -O3 $(PROF) $(CFLAGS_EXTRA)

.PHONY: clean all

all: c c++

c: clean
	rm -rf *.gc*
	cc unit_test.c -o unit_test $(CFLAGS) && ./unit_test
	cc -m32 unit_test.c -o unit_test $(CFLAGS) && ./unit_test
	gcov -a unit_test.c

c++: clean
	rm -rf *.gc*
	g++ unit_test.c -o unit_test $(CFLAGS) && ./unit_test
	gcov -a unit_test.c

w:
	wine cl /DEBUG unit_test.c && wine unit_test.exe

clean:
	rm -rf *.gc* *.dSYM unit_test unit_test.exe
