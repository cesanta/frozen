#!/bin/bash -eu
# Supply build instructions
# Use the following environment variables to build the code
# $CXX:               c++ compiler
# $CC:                c compiler
# CFLAGS:             compiler flags for C files
# CXXFLAGS:           compiler flags for CPP files
# LIB_FUZZING_ENGINE: linker flag for fuzzing harnesses
$CC $CFLAGS -c frozen.c -o frozen.o

# Copy all fuzzer executables to $OUT/
$CC $CFLAGS $LIB_FUZZING_ENGINE \
  $SRC/frozen/.clusterfuzzlite/json_walk_fuzzer.c \
  -o $OUT/json_walk_fuzzer \
  frozen.o \
  -I$PWD
