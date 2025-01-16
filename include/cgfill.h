/**
 * @file cgfill.h
 * @author Jupiter Westbard
 * @date 1/15/2025
 * @brief circlegen coloring function headers
 */

#include <cairo.h>
#include <librsvg/rsvg.h>

#include <cstdint>

#pragma once

struct dpixmap {
    uint8_t *data;
    int width;
    int height;
    int stride;
}; typedef struct dpixmap dpixmap;

/**
 * @brief get pixmap from svg file
 * @param filename input svg
 * @return dpixmap 
 */
dpixmap getSVGColorMap(const char *filename);
