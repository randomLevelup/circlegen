/**
 * @file cgproc.h
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief circlegen processing function headers
 */

#include <tuple>
#include <sstream>
#include <Eigen/Core>

#ifndef CGPROC_H
#define CGPROC_H

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
 * @brief optimization object for circle fitting
 */
struct CircleOptimization {
    const dpoint *pointArray;
    unsigned numPoints;
    unsigned startIndex;
    CircleOptimization() = default;
    CircleOptimization(const dpoint *points, unsigned numPoints) : pointArray(points), numPoints(numPoints), startIndex(0) {}
    double operator()(const Eigen::VectorXd &params, Eigen::VectorXd &) const;
};

/**
 * @brief generate and optimize circles
 * @param points point list to fit circles to
 * @param num number of circles
 * @param sf scale factor for circles
 * @param verbosity level of verbosity for optimizer
 * @return vector of n circles
 */
std::tuple<std::vector<dcircle>, dpointlist> generateCircles(dpointlist &points, int num, float sf, int verbosity);

#endif
