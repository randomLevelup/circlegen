/**
 * @file cgio.h
 * @author Jupiter Westbard
 * @date 03/20/2025
 * @brief defs/headers for circlegen
 */

#include <tuple>
#include <vector>

#ifndef WASM_CIRCLEGEN_H
#define WASM_CIRCLEGEN_H

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

/**
 * @brief Apply a jittered sampling to the image
 * @param pm a dpixmap
 * @param tgtwidth target width
 * @param jitter the jitter factor
 * @return sampled dpixmap
 */
void jitteredResample(dpixmap *pm, int new_width, double jitter);

dpixmap sobelFilter(dpixmap pm);

dpointlist samplePoints(dpixmap pm, int num, double threshold);

std::vector<dcircle> generateCircles(dpointlist &pointlist, dpixmap *pm, int num);

dpixmap quantizeColors(const dpixmap &pm, std::vector<dcircle> &circles);

#endif
