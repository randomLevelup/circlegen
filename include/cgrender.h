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
 * @brief render circles and points and save as "output.jpg"
 * @param bundle dbundle of circles and points
 * @param w image width
 * @param h image height
 * @param sf scale factor (integer)
 */
void renderImage(const dbundle &bundle, dpixmap colors, const float w, const float h, const int sf);

#endif
