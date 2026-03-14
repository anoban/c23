// clang .\argptrs.c -Wall -O3 -Wextra -pedantic -std=c23

// recompiling after renaming the same source to .cpp gives hard compile time errors for const incorrectness!
// clang .\argptrs.cpp -Wall -O3 -Wextra -pedantic -std=c++20

#include <stdio.h>

// dblptr - a constant pointer to constant double
// fltptr - a constant pointer to a constant float
static inline void funcv1(const double* const dblptr, const float* const fltptr) {
    // fltptr = dblptr = NULL; Error: expression must be a modifiable lvalue
    printf("dblptr = %08X, fltptr = %08X\n", dblptr, fltptr);
}

// dblptr - a pointer to double
// fltptr - a pointer to float
static inline void funcv2(double* dblptr, float* fltptr) {
    *fltptr = *dblptr = 10.000; // we can mutate the values
    fltptr = dblptr = NULL;
    printf("dblptr = %08X, fltptr = %08X\n", dblptr, fltptr);
    // and the pointers, but the pointers as passed as values, so this practically has no effect outside this function
}

// dblptr - a constant pointer to double
// fltptr - a constant pointer to float
static inline void funcv3(double* const dblptr, float* const fltptr) {
    *fltptr = *dblptr = 5.0000; // we can mutate the values but not the pointers themselves
    // fltptr = dblptr = NULL; Error: expression must be a modifiable lvalue
    printf("dblptr = %08X, fltptr = %08X\n", dblptr, fltptr);
}

// dblptr - a pointer to constant double
// fltptr - a pointer to a constant float
static inline void funcv4(const double* dblptr, const float* fltptr) {
    // *fltptr = *dblptr = 0.0000; Error: expression must be a modifiable lvalue
    fltptr = dblptr = NULL;
    printf("dblptr = %08X, fltptr = %08X\n", dblptr, fltptr);
}

[[nodiscard]] extern double sum();

int main(void) {
    double       mdbl = 100.00000;
    float        mflt = 100.00000F;
    const double cdbl = 100.00000;
    const float  cflt = 100.00000F;

    printf("mdbl = %4.4lf, mflt = %4.4f, cdbl = %4.4lf, cflt = %4.4f\n", mdbl, mflt, cdbl, cflt);

    funcv1(&cdbl, &cflt); // const type* const
    funcv1(&mdbl, &cflt);
    printf("mdbl = %4.4lf, mflt = %4.4f, cdbl = %4.4lf, cflt = %4.4f\n", mdbl, mflt, cdbl, cflt);

    // clang, cl & gcc all gave warnings only
    // warning: passing 'const float *' to parameter of type 'float *' discards qualifiers
    funcv2(&cdbl, &cflt); // type*
    funcv2(&mdbl, &mflt);
    printf("mdbl = %4.4lf, mflt = %4.4f, cdbl = %4.4lf, cflt = %4.4f\n", mdbl, mflt, cdbl, cflt);

    funcv3(&cdbl, &cflt); // type* const
    funcv3(&mdbl, &mflt);
    printf("mdbl = %4.4lf, mflt = %4.4f, cdbl = %4.4lf, cflt = %4.4f\n", mdbl, mflt, cdbl, cflt);

    funcv4(&cdbl, &cflt); // const type*
    funcv4(&mdbl, &mflt);
    printf("mdbl = %4.4lf, mflt = %4.4f, cdbl = %4.4lf, cflt = %4.4f\n", mdbl, mflt, cdbl, cflt);

    return 0;
}
