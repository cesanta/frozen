PROF = -fprofile-arcs -ftest-coverage -g -O0
CFLAGS = -W -Wall -pedantic -O3 $(CFLAGS_EXTRA)

all:
	gcc $(PROF) unit_test.c -o unit_test $(CFLAGS) && ./unit_test
	gcov -a unit_test
	lcov -d . -t 'unit_test' -o 'unit_test.info' -b . -c
	genhtml -o result unit_test.info

w:
	wine cl unit_test.c && wine unit_test.exe

clean:
	rm -rf  *.info* *.gc* *.dSYM unit_test unit_test.exe

open:
	open result/index.html