GCC = /usr/bin/gcc

CLANG = /usr/bin/clang

ICX = /opt/intel/oneapi/compiler/latest/bin/icx

DEBUG = -DDEBUG -D_DEBUG -g3

NODEBUG = -NDEBUG -N_DEBUG -g0

CFLAGS = -std=c23 -fstrict-flex-arrays -fdiagnostics-show-location=every-line -fdiagnostics-color -Wall -Wextra -Wpedantic

TARGET:

build:
	$(GCC) $(TARGET).c $(CFLAGS) $(NODEBUG) -o $(TARGET).out

.PHONY: clean

clean:
	rm -f ./*.out
	rm -f ./*.o
