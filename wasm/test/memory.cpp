#include <emscripten.h>

int main() { return 0; }

extern "C" {
EMSCRIPTEN_KEEPALIVE
int accumulate(int *arr, int n) {
    int sum = 0;
    while (n) {
        sum += arr[--n];
    }
    return sum;
}

EMSCRIPTEN_KEEPALIVE
const char *get_string() {
    return "Hello, World!";
}

EMSCRIPTEN_KEEPALIVE
void *wasmmalloc(size_t size) {
    return malloc(size);
}

EMSCRIPTEN_KEEPALIVE
void wasmfree(void *ptr) {
    free(ptr);
}
}
