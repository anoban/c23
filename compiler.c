#include <stdio.h>
#include <stdlib.h>

static inline void __attribute__((__always_inline__)) __compiler() {
#if defined(__INTEL_LLVM_COMPILER)
    printf("__INTEL_LLVM_COMPILER = %d\n", __INTEL_LLVM_COMPILER);
#endif
#if defined(__llvm__) && defined(__clang__)
    printf("__llvm__ = %d, __clang__ = %d\n", __llvm__, __clang__);
#endif
#if defined(__GNUC__)
    printf("__GNUC__ = %d\n", __GNUC__);
#endif
}

int main(void) {
#if defined(__INTEL_LLVM_COMPILER)
    puts("oneAPI");
#elif defined(__llvm__) && defined(__clang__)
    puts("LLVM");
#elif defined(__GNUC__)
    puts("GCC");
#endif

    __compiler();

    return EXIT_SUCCESS;
}
