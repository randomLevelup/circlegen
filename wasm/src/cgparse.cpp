/**
 * @file cgparse.cpp
 * @author Jupiter Westbard
 * @date 3/21/2025
 * @brief circlegen parsing implementations
 */

#include "wasm_circlegen.h"

#include <iostream>
#include <random>

void jitteredResample(dpixmap *pm, int new_width, double jitter) {
    double scalefactor = (double)new_width / (double)(pm->width);
    int new_height = (int)(pm->height * scalefactor);
    dpixel *new_data = new dpixel[new_width * new_height];
    
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
            
            // Clamp to image boundaries
            orig_x = std::max(0.0, std::min(orig_x, pm->width - 1.0));
            orig_y = std::max(0.0, std::min(orig_y, pm->height - 1.0));
            
            // Get the four surrounding pixels for bilinear interpolation
            int x0 = (int)orig_x;
            int y0 = (int)orig_y;
            int x1 = std::min(x0 + 1, pm->width - 1);
            int y1 = std::min(y0 + 1, pm->height - 1);
            
            // Calculate interpolation weights
            double wx1 = orig_x - x0;
            double wy1 = orig_y - y0;
            double wx0 = 1.0 - wx1;
            double wy0 = 1.0 - wy1;
            
            // Get the four surrounding pixels
            dpixel p00 = pm->data[y0 * pm->width + x0];
            dpixel p01 = pm->data[y0 * pm->width + x1];
            dpixel p10 = pm->data[y1 * pm->width + x0];
            dpixel p11 = pm->data[y1 * pm->width + x1];
            
            // Bilinear interpolation for each color channel
            dpixel result;
            result.R = (uint8_t)(
                wx0 * wy0 * p00.R +
                wx1 * wy0 * p01.R +
                wx0 * wy1 * p10.R +
                wx1 * wy1 * p11.R
            );
            
            result.G = (uint8_t)(
                wx0 * wy0 * p00.G +
                wx1 * wy0 * p01.G +
                wx0 * wy1 * p10.G +
                wx1 * wy1 * p11.G
            );
            
            result.B = (uint8_t)(
                wx0 * wy0 * p00.B +
                wx1 * wy0 * p01.B +
                wx0 * wy1 * p10.B +
                wx1 * wy1 * p11.B
            );
            
            // Store the result
            new_data[y * new_width + x] = result;
        }
    }
    
    // Clean up old data and update the pixmap
    delete[] pm->data;
    pm->data = new_data;
    pm->width = new_width;
    pm->height = new_height;
}
