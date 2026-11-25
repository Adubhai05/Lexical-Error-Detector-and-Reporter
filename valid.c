#include <stdio.h>
#define MAX 100

/* Simple program: sum of squares and a struct demo */
struct Point { int x; int y; };

int square(int n) {
    return n * n;
}

int main(void) {
    int sum = 0;
    unsigned int mask = 0xFF;
    float ratio = 3.14f, small = 1e-3;
    char letter = 'A', newline = '\n';
    struct Point p = {1, 2};
    struct Point *ptr = &p;

    for (int i = 0; i < MAX; i++) {   // loop over the range
        sum += square(i);
        if (sum >= 1000 && i != 5 || ptr->x == 1) {
            sum >>= 1;
        }
    }
    printf("Sum = %d, ratio = %.2f\n", sum, ratio);
    return 0;
}
