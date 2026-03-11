#include <stdio.h>

int main(void) {
    int i = 5;
    printf("%d\n", i);
    i = ++i + ++i;
    printf("%d\n", i);
    i = ++i + i++;
    printf("%d\n", i);
}
