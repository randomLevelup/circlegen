/**
 * @file cgparse.cpp
 * @author Jupiter Westbard
 * @date 3/21/2025
 * @brief circlegen parsing implementations
 */

#include "wasm_circlegen.h"

#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>
#include <cstdlib>

void formatAlpha(dpixmap *pm) {
    // remove alpha channel (will resize the data array)
    int width = pm->width;
    int height = pm->height;
    uint8_t *data = pm->data; // Keep pointer to the input data
    uint8_t *new_data = new uint8_t[width * height * 3];

    for (int i = 0; i < width * height; ++i) {
        new_data[i * 3] = data[i * 4];     // R
        new_data[i * 3 + 1] = data[i * 4 + 1]; // G
        new_data[i * 3 + 2] = data[i * 4 + 2]; // B
    }

    // Use free instead of delete[] for malloc'd buffers from JavaScript
    free(data);
    // Update the pixmap to point to the new data buffer
    pm->data = new_data;
}

void jitteredResample(dpixmap *pm, int new_width, double jitter) {
    // If dimensions don't change, maybe return early?
    if (new_width == pm->width) {
         // Check height too if necessary
         // return; // Or just proceed if jitter effect is always desired
    }

    double scalefactor = (double)new_width / (double)(pm->width);
    int new_height = (int)std::floor(pm->height * scalefactor);

    std::cout << "Resampling from " << pm->width << "x" << pm->height
              << " to " << new_width << "x" << new_height
              << " with jitter: " << jitter << std::endl;

    // Allocate NEW buffer for the resampled data
    uint8_t *new_data = new uint8_t[new_width * new_height * 3];

    // Store original properties for sampling
    uint8_t *original_data = pm->data; // Keep pointer to the input data
    int original_width = pm->width;
    int original_height = pm->height; // Need original height for clamping

    // Create random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> jitter_dist(-jitter, jitter);
    
    // For each pixel in the new image
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            // Calculate the corresponding position in the original image
            double orig_x = x / scalefactor;
            double orig_y = y / scalefactor;
            
            // Add jitter
            if (jitter > 0.0) {
                orig_x += jitter_dist(gen);
                orig_y += jitter_dist(gen);
            }
            
            // Clamp to original image boundaries
            orig_x = std::max(0.0, std::min(orig_x, original_width - 1.0));
            orig_y = std::max(0.0, std::min(orig_y, original_height - 1.0));
            
            // Get the four surrounding pixels for bilinear interpolation
            int x0 = (int)orig_x;
            int y0 = (int)orig_y;
            int x1 = std::min(x0 + 1, original_width - 1);
            int y1 = std::min(y0 + 1, original_height - 1);
            
            // Calculate interpolation weights
            double wx1 = orig_x - x0;
            double wy1 = orig_y - y0;
            double wx0 = 1.0 - wx1;
            double wy0 = 1.0 - wy1;
            
            // Get the four surrounding pixels from the ORIGINAL data buffer
            // Use original_width for indexing
            size_t p00_idx = (size_t)y0 * original_width * 3 + x0 * 3;
            size_t p01_idx = (size_t)y0 * original_width * 3 + x1 * 3;
            size_t p10_idx = (size_t)y1 * original_width * 3 + x0 * 3;
            size_t p11_idx = (size_t)y1 * original_width * 3 + x1 * 3;
            
            // Bilinear interpolation for each color channel
            new_data[(y * new_width + x) * 3] = (uint8_t)(
                wx0 * wy0 * original_data[p00_idx] +
                wx1 * wy0 * original_data[p01_idx] +
                wx0 * wy1 * original_data[p10_idx] +
                wx1 * wy1 * original_data[p11_idx]
            );
            
            new_data[(y * new_width + x) * 3 + 1] = (uint8_t)(
                wx0 * wy0 * original_data[p00_idx + 1] +
                wx1 * wy0 * original_data[p01_idx + 1] +
                wx0 * wy1 * original_data[p10_idx + 1] +
                wx1 * wy1 * original_data[p11_idx + 1]
            );
            
            new_data[(y * new_width + x) * 3 + 2] = (uint8_t)(
                wx0 * wy0 * original_data[p00_idx + 2] +
                wx1 * wy0 * original_data[p01_idx + 2] +
                wx0 * wy1 * original_data[p10_idx + 2] +
                wx1 * wy1 * original_data[p11_idx + 2]
            );                        
        }
    }
    
    // Free the input data buffer (passed via pm->data)
    delete[] original_data;

    // Update the pixmap structure
    pm->data = new_data;
    pm->width = new_width;
    pm->height = new_height;
}
