#include <emscripten.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include "wasm_circlegen.h"

// Global variables to store output dimensions
static int g_outputWidth = 0;
static int g_outputHeight = 0;

EMSCRIPTEN_KEEPALIVE
extern "C" int getOutputWidth() {
    return g_outputWidth;
}

EMSCRIPTEN_KEEPALIVE
extern "C" int getOutputHeight() {
    return g_outputHeight;
}

EMSCRIPTEN_KEEPALIVE
extern "C" uint8_t *processImageData(uint8_t *data, int width, int height, int numCircles, int drawLines) {
    // Reset global dimensions at start
    g_outputWidth = 0;
    g_outputHeight = 0;
    
    // Test printf output at the start
    fprintf(stdout, "Starting image processing...\n"); fflush(stdout);
    
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
    // Create a proper copy for resampling
    dpixmap resampledPm;
    resampledPm.width = inputPm.width;
    resampledPm.height = inputPm.height;
    // Allocate new memory for the copy
    size_t rgbSize = (size_t)inputPm.width * inputPm.height * 3;
    resampledPm.data = new uint8_t[rgbSize];
    // Copy the RGB data
    memcpy(resampledPm.data, inputPm.data, rgbSize);
    
    // Limit processing width to match JS max width 1000
    int desiredWidth = (width > 1000) ? 1000 : width;
    jitteredResample(&resampledPm, desiredWidth, 0.75);

    printf("Running filters [sobel]...\n"); fflush(stdout);
    dpixmap filtered = sobelFilter(resampledPm); // Allocates filtered.data
    printf("Filtering complete\n\n"); fflush(stdout);

    printf("Sampling points...\n"); fflush(stdout);
    dpointlist points = samplePoints(filtered, 300, 0.75);
    printf("Sampling done\n\n"); fflush(stdout);

    printf("Generating circles...\n"); fflush(stdout);
    std::vector<dcircle> circles = generateCircles(points, &resampledPm, numCircles);
    printf("Circle generation done\n\n"); fflush(stdout);

    printf("Quantizing colors...\n"); fflush(stdout);
    // Allocates outputPm.data (RGB) using dimensions from resampled inputPm
    // Use resampled image dimensions for color quantization
    dpixmap outputPm = quantizeColors(resampledPm, circles, drawLines != 0);
    printf("Color quantization complete\n\n"); fflush(stdout);

    printf("Drawing final image...\n"); fflush(stdout);
    // Use outputPm dimensions for allocation (should match resampled inputPm dimensions)
    int outputWidth = outputPm.width;
    int outputHeight = outputPm.height;
    
    printf("Output dimensions: %dx%d\n", outputWidth, outputHeight); fflush(stdout);
    
    size_t outputNumBytesRGBA = (size_t)outputWidth * outputHeight * 4;
    uint8_t *result_rgba = new uint8_t[outputNumBytesRGBA];

    // Copy RGB data from outputPm to RGBA buffer
    for (int i = 0; i < outputWidth * outputHeight; i++) {
        result_rgba[i * 4] = outputPm.data[i * 3];     // R
        result_rgba[i * 4 + 1] = outputPm.data[i * 3 + 1]; // G
        result_rgba[i * 4 + 2] = outputPm.data[i * 3 + 2]; // B
        result_rgba[i * 4 + 3] = 255;                      // Alpha
    }

    // Store output dimensions for retrieval by JS (set at the end to ensure accuracy)
    g_outputWidth = outputWidth;
    g_outputHeight = outputHeight;

    // Free intermediate buffers
    delete[] inputPm.data;   // Free the buffer allocated by formatAlpha
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

