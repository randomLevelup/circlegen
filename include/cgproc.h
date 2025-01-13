/**
 * @file cgproc.h
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief circlegen processing function headers
 */

#include <tuple>

#pragma once

typedef std::vector<std::tuple<float, float>> dpointlist;
typedef std::tuple<float, float, float> dcircle;

/**
 * @brief point object for cacheability
 */
struct dpoint {
    double x;
    double y;
}; typedef struct dpoint dpoint;

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
std::tuple<std::vector<dcircle>, dpointlist> generateCircles(dpointlist &points, int num);
