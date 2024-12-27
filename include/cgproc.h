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
    int closest_circle;
}; typedef struct dpoint dpoint;

struct dbounds {
    float x_min;
    float x_max;
    float y_min;
    float y_max;
}; typedef struct dbounds dbounds;

/**
 * @brief optimization object for circle fitting
 */
struct CircleOptimization {
    const dpoint *pointArray;
    unsigned numPoints;
    CircleOptimization() = default;
    CircleOptimization(const dpoint *points, unsigned numPoints) : pointArray(points), numPoints(numPoints) {}
    double operator()(const Eigen::VectorXd &params, Eigen::VectorXd &grad) const;
};

/**
 * @brief generate and optimize circles
 * @param points point list to fit circles to
 * @param num number of circles
 * @return vector of n circles
 */
std::vector<dcircle> generateCircles(dpointlist &points, int num);

/**
 * @brief clean up extraneous circles
 * @param circles list of generated circles
 */
void cleanCircles(dpointlist &points, std::vector<dcircle> &circles);
