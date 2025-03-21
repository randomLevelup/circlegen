/**
 * @file cgio.h
 * @author Jupiter Westbard
 * @date 03/20/2025
 * @brief defs/headers for circlegen
 */

#include <tuple>
#include <vector>

#ifndef CIRCLEGEN_H
#define CIRCLEGEN_H

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

typedef std::tuple<int, int> dpoint;
typedef std::vector<dpoint> dpointlist;
typedef std::tuple<double, double, double> dcircle;

bool equalCircles(const dcircle &lhs, const dcircle &rhs, double epsilon);

/**
 * @brief Parse an image file and return a dpixmap structure
 * @param filename Path to the image file
 * @return dpixmap structure containing image data
 */
dpixmap parseImage(const char *filename);

/**
 * @brief Apply a jittered sampling to the image
 * @param pm a dpixmap
 * @param tgtwidth target width
 * @param jitter the jitter factor
 * @return sampled dpixmap
 */
void jitteredResample(dpixmap *pm, int new_width, double jitter);

/**
 * @brief Save a dpixmap structure to an image file
 * @param pm dpixmap structure containing image data
 * @param points (optional) List of points to be saved
 */
void saveImage(dpixmap pm, dpointlist *points);

void breakpointSaveImage(dpixmap *pm, dpointlist &points, dcircle &current, dcircle &last);

dpixmap sobelFilter(dpixmap pm);

dpointlist samplePoints(dpixmap pm, int num, double threshold);

std::vector<dcircle> generateCircles(dpointlist &pointlist, dpixmap *pm, int num);


#endif
