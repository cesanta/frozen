PROF = -fprofile-arcs -ftest-coverage -g -O0
CFLAGS = -W -Wall -pedantic -O3 $(CFLAGS_EXTRA)

all:
	cc unit_test.c -o unit_test $(CFLAGS) && ./unit_test
	g++ unit_test.c -o unit_test $(CFLAGS) && ./unit_test
	gcov -a unit_test.c

w:
	wine cl unit_test.c && wine unit_test.exe

clean:
	rm -rf *.gc* *.dSYM unit_test unit_test.exe
