/**
 * @file cgfill.h
 * @author Jupiter Westbard
 * @date 1/15/2025
 * @brief circlegen coloring function headers
 */

#include <cairo.h>
#include <librsvg/rsvg.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "cgproc.h"

#ifndef CGFILL_H
#define CGFILL_H

struct dpixmap_ {
    uint8_t *data;
    int width;
    int height;
    int stride;
    int scalefactor;
}; typedef struct dpixmap_ dpixmap;

struct dpixel_ {
    uint8_t rgb[3];
    uint8_t a;
    int idx;
}; typedef struct dpixel_ dpixel;

struct overlapgroup_ {
    std::unordered_map<uint32_t, std::vector<dpixel>> table;
    std::vector<uint32_t> keys;
}; typedef struct overlapgroup_ overlapgroup;


dpixmap quantizeColors(const dpixmap &pm, std::vector<dcircle> &circles, float c_sf);


/**
 * @brief get pixmap from svg file
 * @param filename input svg
 * @return dpixmap 
 */
dpixmap getSVGColorMap(const char *filename, float minX, float minY);

#endif
