/**
 * @file cgrender.h
 * @author Jupiter Westbard
 * @date 1/19/2025
 * @brief circlegen rendering function header
 */

#include "cgiosvg.h"
#include "cgfill.h"

#ifndef CGRENDER_H
#define CGRENDER_H
/**
 * @brief Render circles and points and save as an image
 * @param bundle A bundle of circles and points
 * @param pb pathbundle for circle-space dimensions dimensions
 * @param colors A pixmap for svg-space dimensions & color info
 * @param rpoints Flag to render points
 * @param rcircles Flag to render circles
 * @param rfill Flag to render fill
 */
void renderImage(const dbundle &bundle, pathbundle &pb, dpixmap &colors,
                 bool rpoints, bool rcircles, bool rfill);

#endif
