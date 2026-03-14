#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

struct align {
        bool               first;
        short              second;
        double             third;
        char               fourth;
        unsigned long long fifth;
};

// https://stackoverflow.com/questions/14332633/attribute-packed-v-s-gcc-attribute-alignedx
struct aligned16 {
        bool               first;
        short              second;
        double             third;
        char               fourth;
        unsigned long long fifth;
} __attribute__((aligned(16)));

int main(void) {
    alignof(struct align);
    __alignof(struct aligned16);
    __alignof__(struct align);
    printf("Hi there %s\n", "Anoban");
    return 0;
}
