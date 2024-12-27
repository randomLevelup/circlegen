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
 * @brief optimization object for circle fitting
 */
struct CircleOptimization {
    dpointlist pointlist;
    CircleOptimization() = default;
    CircleOptimization(const dpointlist &points) : pointlist(points) {}
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
