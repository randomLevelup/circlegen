#include <emscripten.h>
#include <vector>
#include <cstdint>
#include "./circlegen.h"

int main() { return 0; }

extern "C" {
EMSCRIPTEN_KEEPALIVE
uint8_t *processImageData(char *data, int width, int height) {
    dpixmap inputPm;
    inputPm.width = width;
    inputPm.height = height;
    inputPm.data = new dpixel[width * height];
    for (int i = 0; i < width * height; i++) {
        inputPm.data[i].R = data[i * 4];
        inputPm.data[i].G = data[i * 4 + 1];
        inputPm.data[i].B = data[i * 4 + 2];
    }

    jitteredResample(&inputPm, std::min(1000, width), 0.75);
    dpixmap filtered = sobelFilter(inputPm);
    dpointlist points = samplePoints(filtered, 300, 0.75);
    std::vector<dcircle> circles = generateCircles(points, &inputPm, 6);
    dpixmap outputPm = quantizeColors(inputPm, circles);

    uint8_t *result = new uint8_t[width * height * 4];
    for (int i = 0; i < width * height; i++) {
        result[i * 4] = outputPm.data[i].R;
        result[i * 4 + 1] = outputPm.data[i].G;
        result[i * 4 + 2] = outputPm.data[i].B;
        result[i * 4 + 3] = 255; // Alpha channel
    }

    delete[] inputPm.data;
    delete[] filtered.data;
    delete[] outputPm.data;

    return result;
}

EMSCRIPTEN_KEEPALIVE
void freeImageData(uint8_t *data) {
    free(data);
}
}
