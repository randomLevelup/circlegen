#include <iostream>
#include <emscripten.h>

extern "C" {
EMSCRIPTEN_KEEPALIVE
int addNums(int a, int b) {
    return a + b;
}
}

extern "C" {
EMSCRIPTEN_KEEPALIVE
int subtractNums(int a, int b) {
    return a - b;
}
}

EMSCRIPTEN_KEEPALIVE
int main() {
    printf("Hello World, %d\n", addNums(1, 2));
    return 0;
}
