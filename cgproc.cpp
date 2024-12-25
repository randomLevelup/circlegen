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

#include <Eigen/Core>
#include "gdcpp.h"

#include "cgproc.h"
#include "cgio.h"

std::vector<dcircle> generateCircles(dpointlist &points, int num);
std::vector<dcircle> makeInitialGuess(dpointlist &points, int num);

struct CircleOptimization {
    double operator()(const Eigen::VectorXd &params, Eigen::VectorXd &grad) const {

        // calculate loss

        //

        // calculate gradient

        //

        return 0.0;
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

    gdc::GradientDescent<double, CircleOptimization, gdc::WolfeBacktracking<double>> optimizer;
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
