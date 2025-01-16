/**
 * @file cgproc.h
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief circlegen processing function headers
 */

#include <tuple>
#include <sstream>

#pragma once

typedef std::vector<std::tuple<float, float>> dpointlist;
typedef std::tuple<float, float, float> dcircle;
typedef std::stringstream PixelStream;

/**
 * @brief point object for cacheability
 */
struct dpoint_ {
    double x;
    double y;
}; typedef struct dpoint_ dpoint;

/**
 * @brief pixel struct for streaming
 */
struct dpixel_ {
    uint8_t r;
    uint8_t g;
    uint8_t b;
}; typedef struct dpixel_ dpixel;

/**
 * @brief optimization object for circle fitting
 */
struct CircleOptimization {
    const dpoint *pointArray;
    unsigned numPoints;
    unsigned startIndex;
    CircleOptimization() = default;
    CircleOptimization(const dpoint *points, unsigned numPoints) : pointArray(points), numPoints(numPoints), startIndex(0) {}
    double operator()(const Eigen::VectorXd &params, Eigen::VectorXd &grad) const;
};

/**
 * @brief generate and optimize circles
 * @param points point list to fit circles to
 * @param num number of circles
 * @return vector of n circles
 */
std::tuple<std::vector<dcircle>, dpointlist> generateCircles(dpointlist &points, int num, float sf);

/**
 * @brief get a stream of rgb pixels from an svg file
 * @param filename 
 * @return A stringstream object of pixels
 */
PixelStream getSVGStream(const char *filename);
