/**
 * @file cgrender.h
 * @author Jupiter Westbard
 * @date 1/19/2025
 * @brief circlegen rendering function header
 */

#include "cgio.h"
#include "cgfill.h"

#ifndef CGRENDER_H
#define CGRENDER_H
/**
 * @brief Render circles and points and save as an image
 * @param bundle A bundle of circles and points
 * @param pb pathbundle for circle-space dimensions dimensions
 * @param colors A pixmap for svg-space dimensions & color info
 */
void renderImage(const dbundle &bundle, pathbundle &pb, dpixmap &colors);

#endif
