/**
 * @file cgproc.h
 * @author Jupiter Westbard
 * @date 03/20/2025
 * @brief circlegen processing function headers
 */

#include "cgio.h"

#ifndef CGPROC_H
#define CGPROC_H

dpixmap sobelFilter(dpixmap pm);

dpointlist samplePoints(dpixmap pm, int num, double threshold);

#endif
