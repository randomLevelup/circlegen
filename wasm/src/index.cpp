#include <emscripten.h>
#include <iostream>
#include <cstdio>
#include "wasm_circlegen.h"

// Add a simple test function for printf
EMSCRIPTEN_KEEPALIVE
extern "C" void testPrintf() {
    printf("Test printf function - if you see this, printf is working!\n");
    fflush(stdout);
}

EMSCRIPTEN_KEEPALIVE
extern "C" uint8_t *processImageData(uint8_t *data, int width, int height) {
    // Test printf output at the start
    fprintf(stdout, "Starting image processing...\n");
    fflush(stdout);
    fprintf(stderr, "This is an error test message\n");
    fflush(stderr);
    
    printf("Loading image %dx%d...\n", width, height); 
    fflush(stdout);
    dpixmap inputPm;
    inputPm.width = width;
    inputPm.height = height;
    // NOTE: 'data' is owned by the caller (JS) initially.
    // formatAlpha and jitteredResample will allocate new buffers
    // and free the one previously pointed to by inputPm.data.
    inputPm.data = data;
    printf("image loaded\n\n"); fflush(stdout);

    printf("Running filters [format]...\n"); fflush(stdout);
    formatAlpha(&inputPm); // Frees original 'data', inputPm.data points to new RGB buffer

    printf("Running filters [dither]...\n"); fflush(stdout);
    // Create a copy for resampling, so the original inputPm is preserved for color quantization.
    dpixmap resampledPm = inputPm;
    resampledPm.data = nullptr; // Avoids double-freeing the original data pointer
    jitteredResample(&resampledPm, width, 0.75);

    printf("Running filters [sobel]...\n"); fflush(stdout);
    dpixmap filtered = sobelFilter(resampledPm); // Allocates filtered.data
    printf("Filtering complete\n\n"); fflush(stdout);

    printf("Sampling points...\n"); fflush(stdout);
    dpointlist points = samplePoints(filtered, 300, 0.75);
    printf("Sampling done\n\n"); fflush(stdout);

    printf("Generating circles...\n"); fflush(stdout);
    std::vector<dcircle> circles = generateCircles(points, &resampledPm, 6);
    printf("Circle generation done\n\n"); fflush(stdout);

    printf("Quantizing colors...\n"); fflush(stdout);
    // Allocates outputPm.data (RGB) using dimensions from resampled inputPm
    dpixmap outputPm = quantizeColors(inputPm, circles);
    printf("Color quantization complete\n\n"); fflush(stdout);

    printf("Drawing final image...\n"); fflush(stdout);
    // Use outputPm dimensions for allocation (should match resampled inputPm dimensions)
    int outputWidth = outputPm.width;
    int outputHeight = outputPm.height;
    size_t outputNumBytesRGBA = (size_t)outputWidth * outputHeight * 4;
    uint8_t *result_rgba = new uint8_t[outputNumBytesRGBA];

    // Copy RGB data from outputPm to RGBA buffer
    for (int i = 0; i < outputWidth * outputHeight; i++) {
        result_rgba[i * 4] = outputPm.data[i * 3];     // R
        result_rgba[i * 4 + 1] = outputPm.data[i * 3 + 1]; // G
        result_rgba[i * 4 + 2] = outputPm.data[i * 3 + 2]; // B
        result_rgba[i * 4 + 3] = 255;                      // Alpha
    }

    // Free intermediate buffers
    delete[] resampledPm.data; // Free the buffer allocated by jitteredResample
    delete[] filtered.data;  // Free the buffer allocated by sobelFilter
    delete[] outputPm.data;  // Free the buffer allocated by quantizeColors

    printf("Generation complete. Terminating\n"); fflush(stdout);
    // Return the final RGBA buffer (to be freed by JS via freeImageData)
    return result_rgba;
}

EMSCRIPTEN_KEEPALIVE
extern "C" void freeImageData(uint8_t *data) {
    // Free the buffer allocated by processImageData and returned to JS
    delete[] data;
}

