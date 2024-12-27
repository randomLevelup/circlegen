/**
 * @file cgproc.cpp
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief circlegen processing implementations
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <tuple>
#include <limits>

#include <Eigen/Core>
#include "gdcpp.h"

#include "cgproc.h"
#include "cgio.h"

static std::vector<dcircle> makeInitialGuess(dpointlist &points, int num, std::string method);
static std::vector<dcircle> makeInitialGuessRandom(dpointlist &points, int num);
static std::vector<dcircle> makeInitialGuessNormal(dpointlist &points, int num);
std::vector<dcircle> generateCircles(dpointlist &points, int num);
void cleanCircles(dpointlist &points, std::vector<dcircle> &circles);

static std::vector<dcircle> makeInitialGuess(dpointlist &points, int num, std::string method = "random") {
    if (method == "random") {
        return makeInitialGuessRandom(points, num);
    } else if (method == "normal") {
        return makeInitialGuessNormal(points, num);
    } else {
        return makeInitialGuessRandom(points, num);
    }
}

static std::vector<dcircle> makeInitialGuessRandom(dpointlist &points, int num) {
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

static std::vector<dcircle> makeInitialGuessNormal(dpointlist &points, int num) {
    // Determine the x and y bounds of all the points
    auto [min_x, max_x] = std::minmax_element(points.begin(), points.end(), 
        [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });
    auto [min_y, max_y] = std::minmax_element(points.begin(), points.end(), 
        [](const auto& a, const auto& b) { return std::get<1>(a) < std::get<1>(b); });

    float x_min = std::get<0>(*min_x);
    float x_max = std::get<0>(*max_x);
    float y_min = std::get<1>(*min_y);
    float y_max = std::get<1>(*max_y);

    std::vector<dcircle> circles;
    float radius = std::min(x_max - x_min, y_max - y_min) / (4 * std::sqrt(num));
    int grid_size = static_cast<int>(std::sqrt(num));
    float x_step = (x_max - x_min) / grid_size;
    float y_step = (y_max - y_min) / grid_size;

    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            if (circles.size() >= (unsigned)num) break;
            float x = x_min + i * x_step + x_step / 2;
            float y = y_min + j * y_step + y_step / 2;
            circles.push_back(std::make_tuple(x, y, radius));
        }
    }

    return circles;
}

std::vector<dcircle> generateCircles(dpointlist &points, int num) {
    std::cout << "making initial guess\n";
    Eigen::VectorXd initialGuess(3 * num);
    std::vector<dcircle> circles = makeInitialGuess(points, num, "random");
    for (int i = 0; i < num; ++i) {
        initialGuess(3 * i) = std::get<0>(circles[i]);
        initialGuess(3 * i + 1) = std::get<1>(circles[i]);
        initialGuess(3 * i + 2) = std::get<2>(circles[i]);
    }

    std::cout << "creating optimizer object\n";
    CircleOptimization optimization(points);
    gdc::GradientDescent<double, CircleOptimization, gdc::WolfeBacktracking<double>> optimizer;
    optimizer.setObjective(optimization);

    std::cout << "setting optimizer parameters\n";
    optimizer.setMaxIterations(150);
    optimizer.setMinGradientLength(1e-6);
    optimizer.setMinStepLength(1e-6);
    optimizer.setMomentum(0.2);
    optimizer.setVerbosity(1);
    std::cout << "optimizer ready\n";

    std::cout << "optimizing (minimizing total circle distances)\n";
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

double CircleOptimization::operator()(const Eigen::VectorXd &params, Eigen::VectorXd &grad) const {
    double loss = 0.0;
    grad.setZero(params.size());

    double px, py, cx, cy, r, dist, min_dist, dcx, dcy, dr;
    for (const auto &point : pointlist) {
        px = std::get<0>(point);
        py = std::get<1>(point);

        min_dist = std::numeric_limits<double>::max();
        int closest_circle = 0;

        for (int j = 0; j < params.size() / 3; ++j) {
            cx = params(3 * j);
            cy = params(3 * j + 1);
            r = params(3 * j + 2);

            dist = std::abs(std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy)) - r) * 4;
            if (dist < min_dist) {
                min_dist = dist;
                closest_circle = j;
            }
        }

        loss += min_dist;

        cx = params(3 * closest_circle);
        cy = params(3 * closest_circle + 1);
        r = params(3 * closest_circle + 2);

        dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
        dcx = ((px - cx) * (dist - r)) / (dist * std::abs(dist - r));
        dcy = ((py - cy) * (dist - r)) / (dist * std::abs(dist - r));
        dr = (r - dist) / std::abs(dist - r);

        grad(3 * closest_circle) += dcx * 4;
        grad(3 * closest_circle + 1) += dcy * 4;
        grad(3 * closest_circle + 2) += dr * 4;
    }
    return loss;
}

void cleanCircles(dpointlist &points, std::vector<dcircle> &circles) {
    const double max_radius = 100.0; // Define a maximum acceptable radius
    const double max_distance = 200.0; // Define a maximum acceptable distance from the point cloud

    auto is_circle_valid = [&](const dcircle &circle) {
        double cx = std::get<0>(circle);
        double cy = std::get<1>(circle);
        double r = std::get<2>(circle);

        if (r > max_radius) return false;

        for (const auto &point : points) {
            double px = std::get<0>(point);
            double py = std::get<1>(point);
            double dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
            if (dist <= max_distance) return true;
        }
        return false;
    };

    circles.erase(std::remove_if(circles.begin(), circles.end(), 
        [&](const dcircle &circle) { return !is_circle_valid(circle); }), circles.end());
}
