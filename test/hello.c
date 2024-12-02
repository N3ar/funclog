#include <stdio.h>
#include <stdlib.h>

int add(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b;
}

// Func call depth test
int mathops(int a, int b) {
    int c, d;
    c = add(a, a);
    d = add(b, b);
    a = sub(c, b);
    b = sub(d, a);
    return a + b;
}

int main() {
    // lib func call
    printf("Welcome to the test program\n");
    
    int x = 3, y = 5;
    int (*operation)(int, int);
    
    // defined func call
    add(x, x);

    // nested func call
    printf("Sum of X and Y: %i\n", add(x,y));

    // Depth test
    mathops(x, y);

    // FuncPtr test
    operation = add;
    printf("Add: %d\n", operation(3, 4));

    // Spam basicblocks to test basicblock logging
    for (int i = 0; i < 2; i = add(i, 1)) {
        if (i)
            add(x, x);
        sub (x, y);
    }

    return 0;
}
