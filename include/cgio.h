/**
 * @file cgio.h
 * @author Jupiter Westbard
 * @date 03/20/2025
 * @brief input/output function headers for circlegen
 */

#include <tuple>
#include <vector>

#ifndef CGIO_H
#define CGIO_H

struct dpixel {
    int R;
    int G;
    int B;
}; typedef struct dpixel dpixel;

struct dpixmap {
    int width;
    int height;
    dpixel *data;
}; typedef struct dpixmap dpixmap;

typedef std::vector<std::tuple<int, int>> dpointlist;

/**
 * @brief Parse an image file and return a dpixmap structure
 * @param filename Path to the image file
 * @return dpixmap structure containing image data
 */
dpixmap parseImage(const char *filename);

/**
 * @brief Save a dpixmap structure to an image file
 * @param pm dpixmap structure containing image data
 * @param points (optional) List of points to be saved
 */
void saveImage(dpixmap pm, dpointlist *points);

#endif
