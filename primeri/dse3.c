#include "stdlib.h"

int f(int x) {
    return x;
}

int main() {
    int x, y;
    x = 2;
    y = f(x);
    x = 7;
    return 0;
}