#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int wmain(const int argc, const wchar_t* const argv[]) {
    wprintf(L"argc = %d\n", argc);
    for (int i = 0; i < argc; ++i) wprintf(L"Argument no %d: %s\n", i, argv[i]);
    return EXIT_SUCCESS;
}
