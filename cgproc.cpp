/**
 * @file cgproc.cpp
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief circlegen processing implementations
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <tuple>
#include <limits>

#include <Eigen/Core>
#include "gdcpp.h"

#include "cgproc.h"
#include "cgio.h"

std::vector<dcircle> generateCircles(dpointlist &points, int num);
std::vector<dcircle> makeInitialGuess(dpointlist &points, int num);

struct CircleOptimization {
    dpointlist pointlist;
    CircleOptimization() = default;
    CircleOptimization(const dpointlist &points) : pointlist(points) {}

    double operator()(const Eigen::VectorXd &params, Eigen::VectorXd &grad) const {
        double loss = 0.0;
        grad.setZero();

        for (const auto &point : pointlist) {
            double px = std::get<0>(point);
            double py = std::get<1>(point);

            double min_dist = std::numeric_limits<double>::max();
            int closest_circle = -1;

            for (int i = 0; i < params.size() / 3; ++i) {
                double cx = params(3 * i);
                double cy = params(3 * i + 1);
                double r = params(3 * i + 2);

                double dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy)) - r;
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_circle = i;
                }
            }

            loss += min_dist * min_dist;

            if (closest_circle != -1) {
                double cx = params(3 * closest_circle);
                double cy = params(3 * closest_circle + 1);
                double r = params(3 * closest_circle + 2);

                double dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
                double dL_ddist = 2 * min_dist;
                double ddist_dcx = (cx - px) / dist;
                double ddist_dcy = (cy - py) / dist;
                double ddist_dr = -1.0;

                grad(3 * closest_circle) += dL_ddist * ddist_dcx;
                grad(3 * closest_circle + 1) += dL_ddist * ddist_dcy;
                grad(3 * closest_circle + 2) += dL_ddist * ddist_dr;
            }
        }

        return loss;
    }
};

std::vector<dcircle> generateCircles(dpointlist &points, int num) {
    Eigen::VectorXd initialGuess(3 * num);
    std::vector<dcircle> circles = makeInitialGuess(points, num);
    for (int i = 0; i < num; ++i) {
        initialGuess(3 * i) = std::get<0>(circles[i]);
        initialGuess(3 * i + 1) = std::get<1>(circles[i]);
        initialGuess(3 * i + 2) = std::get<2>(circles[i]);
    }

    CircleOptimization optimization(points);
    gdc::GradientDescent<double, CircleOptimization, gdc::WolfeBacktracking<double>> optimizer;
    optimizer.setObjective(optimization);
    optimizer.setMaxIterations(1000);
    optimizer.setMinGradientLength(1e-6);
    optimizer.setMinStepLength(1e-6);
    optimizer.setMomentum(0.4);
    optimizer.setVerbosity(4);

    auto result = optimizer.minimize(initialGuess);

    std::cout << "optimization completed!\n";
    std::cout << "Converged: " << (result.converged ? "true" : "false") << std::endl;
    std::cout << "Iterations: " << result.iterations << std::endl;
    std::cout << "Final loss value: " << result.fval << std::endl;
    std::cout << "Final parameters: " << result.xval.transpose() << std::endl;

    std::vector<dcircle> optimizedCircles;
    for (int i = 0; i < num; ++i) {
        optimizedCircles.emplace_back(result.xval(3 * i), result.xval(3 * i + 1), result.xval(3 * i + 2));
    }

    return optimizedCircles;
}

std::vector<dcircle> makeInitialGuess(dpointlist &points, int num) {
    // Determine the x and y bounds of all the points
    auto [min_x, max_x] = std::minmax_element(points.begin(), points.end(), 
        [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });
    auto [min_y, max_y] = std::minmax_element(points.begin(), points.end(), 
        [](const auto& a, const auto& b) { return std::get<1>(a) < std::get<1>(b); });

    float x_min = std::get<0>(*min_x);
    float x_max = std::get<0>(*max_x);
    float y_min = std::get<1>(*min_y);
    float y_max = std::get<1>(*max_y);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_x(x_min, x_max);
    std::uniform_real_distribution<> dis_y(y_min, y_max);
    std::uniform_real_distribution<> dis_r(0, std::min(x_max - x_min, y_max - y_min) / 2);

    // Add num randomly-chosen circles that would fit within those bounds
    std::vector<dcircle> circles;
    for (int i = 0; i < num; ++i) {
        float x = dis_x(gen);
        float y = dis_y(gen);
        float r = dis_r(gen);
        circles.push_back(std::make_tuple(x, y, r));
    }

    return circles;
}
