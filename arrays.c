#include <stdint.h>
#include <stdio.h>

long double signed_array_sum(char* array, unsigned long long size, unsigned char stride) {
    long double sum = 0.0;

    switch (stride) {
        case 8 :
            long long* i64array = (long long*) array;
            for (unsigned long long i = 0; i < size; ++i) {
                sum += *i64array;
                i64array++;
            }
            break;
        case 4 :
            int* i32array = (int*) array;
            for (unsigned long long i = 0; i < size; ++i) {
                sum += *i32array;
                i32array++;
            }
            break;
        case 2 :
            short* i16array = (short*) array;
            for (unsigned long long i = 0; i < size; ++i) {
                sum += *i16array;
                i16array++;
            }
            break;
        case 1 :
            unsigned char* i8array = (unsigned char*) array;
            for (unsigned long long i = 0; i < size; ++i) {
                sum += *i8array;
                i8array++;
            }
            break;
    }

    return sum;
}

int main(void) {
    int array[]     = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

    long double sum = 0.0;
    for (unsigned char stride = 1; stride < 9; stride *= 2) {
        sum = signed_array_sum((char*) array, 10, stride);
        printf("Sum %10.5Lf when stride is %u\n", sum, stride);
    }

    return 0;
}
