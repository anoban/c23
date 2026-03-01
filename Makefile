GCC = /usr/bin/gcc
CLANG = /usr/bin/clang
ICX = /opt/intel/oneapi/compiler/latest/bin/icx

TARGET_NO_EXTENSION = asciiart

SOURCE_EXTENSION = c

FLAGS = -std=c23 -fstrict-flex-arrays -fdiagnostics-show-location=every-line -fdiagnostics-color -Wall -Wextra -Wpedantic

build:
	$(GCC) $(TARGET_NO_EXTENSION).$(SOURCE_EXTENSION) $(FLAGS) -o $(TARGET_NO_EXTENSION).out

.PHONY: clean

clean:
	rm -f ./*.out
	rm -f ./*.o
