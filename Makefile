# Makefile for gcc / MinGW / WSL / Linux users.
#   On Windows with MSVC only, use:  build.bat
#
#   make test        build and run all three suites
#   make benchmark   build the benchmark, then: ./bench-bin 100000
#   make asan        build the linked-list suite with AddressSanitizer
#   make clean

CC       ?= gcc
CFLAGS   ?= -std=c11 -Wall -Wextra -O2 -g
CPPFLAGS ?= -Iinclude

CORE  := src/task.c src/seqlist.c src/linkedlist.c
TESTS := tests/test_list.c

# On Windows the binaries get a .exe suffix and cmd does not accept "./name".
ifeq ($(OS),Windows_NT)
EXE := .exe
RUN :=
else
EXE :=
RUN := ./
endif

.PHONY: all test test-seq test-linked test-cross benchmark asan clean

all: test

test: test-seq test-linked test-cross
	@$(RUN)test-seq$(EXE)
	@$(RUN)test-linked$(EXE)
	@$(RUN)test-cross$(EXE)
	@echo "== all three suites passed =="

test-seq: $(CORE) $(TESTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DUSE_SEQLIST -o test-seq$(EXE) $(CORE) $(TESTS)

test-linked: $(CORE) $(TESTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DUSE_LINKEDLIST -o test-linked$(EXE) $(CORE) $(TESTS)

test-cross: $(CORE) tests/test_cross.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o test-cross$(EXE) $(CORE) tests/test_cross.c

benchmark: $(CORE) bench/bench.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -Ibench -o bench-bin$(EXE) $(CORE) bench/bench.c

# Leak / use-after-free detection. Works on Linux, WSL and most gcc builds.
# (MinGW usually ships without ASan; use MSVC's /fsanitize=address in VS instead.)
asan:
	$(CC) -std=c11 -g -O1 -fsanitize=address -fno-omit-frame-pointer \
	      $(CPPFLAGS) -DUSE_LINKEDLIST -o test-asan$(EXE) $(CORE) $(TESTS)
	$(RUN)test-asan$(EXE)

clean:
	rm -f test-seq$(EXE) test-linked$(EXE) test-cross$(EXE) test-asan$(EXE) bench-bin$(EXE)
	rm -f *.o *.obj *.exe *.tmp
	rm -rf build
