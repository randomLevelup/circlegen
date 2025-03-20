/**
 * @file cgproc.cpp
 * @author Jupiter Westbard
 * @date 03/20/2025
 * @brief circlegen processing implementations
 */

#include <vector>
#include <tuple>
#include <cmath>
#include <random>
#include <algorithm>

#include "cgio.h"
#include "cgproc.h"

dpixmap sobelFilter(dpixmap pm) {
    dpixmap filtered = {pm.width, pm.height, new dpixel[pm.width * pm.height]};
    int width = pm.width;
    int height = pm.height;

    // Sobel kernels
    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int Gy[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int sumX = 0;
            int sumY = 0;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixelValue = pm.data[(y + ky) * width + (x + kx)].R;
                    sumX += pixelValue * Gx[ky + 1][kx + 1];
                    sumY += pixelValue * Gy[ky + 1][kx + 1];
                }
            }

            int magnitude = static_cast<int>(std::sqrt(sumX * sumX + sumY * sumY));
            if (magnitude > 255) magnitude = 255;

            filtered.data[y * width + x].R = magnitude;
            filtered.data[y * width + x].G = magnitude;
            filtered.data[y * width + x].B = magnitude;
        }
    }

    return filtered;
}

static double mag_factor(dpixel pixel) {
    double mag = sqrt(pixel.R * pixel.R + pixel.G * pixel.G + pixel.B * pixel.B);
    return (255.0 - mag) / 255.0;
}

dpointlist samplePoints(dpixmap pm, int num, double threshold) {
    dpointlist points;

    for (int i = 0; i < (pm.width * pm.height); ++i) {
        double mag = mag_factor(pm.data[i]);
        if (mag < threshold) {
            int x = i % pm.width;
            int y = i / pm.width;
            points.push_back(std::make_tuple(x, y));
        }
    }

    // shuffle the points
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(points.begin(), points.end(), g);

    // pick up to num points
    if (points.size() > num) {
        points.resize(num);
    }
    return points;
}
