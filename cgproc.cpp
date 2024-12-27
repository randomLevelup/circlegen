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

static dpoint *loadPointArray(const dpointlist &pointlist);
static std::vector<dcircle> makeInitialGuess(dpointlist &pointlist, int num, std::string method);
static std::vector<dcircle> makeInitialGuessRandom(int numCircles, dbounds b);
static std::vector<dcircle> makeInitialGuessNormal(int numCircles, dbounds b);
static void initializeDistances(const std::vector<dcircle> &circles, dpoint *pointArray, int numPoints);
std::vector<dcircle> generateCircles(dpointlist &pointlist, int num);
void cleanCircles(dpointlist &points, std::vector<dcircle> &circles);

static dpoint *loadPointArray(const dpointlist &pointlist) {
    dpoint *pointarray = new dpoint[pointlist.size()];
    for (unsigned i = 0; i < pointlist.size(); ++i) {
        pointarray[i].x = std::get<0>(pointlist[i]);
        pointarray[i].y = std::get<1>(pointlist[i]);
        pointarray[i].closest_circle = -1; // Initialize closest_circle
    }
    return pointarray;
}

static std::vector<dcircle> makeInitialGuess(dpointlist &pointlist, int num, std::string method) {
    auto [min_x, max_x] = std::minmax_element(pointlist.begin(), pointlist.end(), 
        [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });
    auto [min_y, max_y] = std::minmax_element(pointlist.begin(), pointlist.end(), 
        [](const auto& a, const auto& b) { return std::get<1>(a) < std::get<1>(b); });

    dbounds bounds;
    bounds.x_min = std::get<0>(*min_x);
    bounds.x_max = std::get<0>(*max_x);
    bounds.y_min = std::get<1>(*min_y);
    bounds.y_max = std::get<1>(*max_y);

    if (method == "random") {
        return makeInitialGuessRandom(num, bounds);
    } else if (method == "normal") {
        return makeInitialGuessNormal(num, bounds);
    } else {
        return makeInitialGuessRandom(num, bounds);
    }
}

static std::vector<dcircle> makeInitialGuessRandom(int numCircles, dbounds b) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_x(b.x_min, b.x_max);
    std::uniform_real_distribution<> dis_y(b.y_min, b.y_max);
    std::uniform_real_distribution<> dis_r(0, std::min(b.x_max - b.x_min, b.y_max - b.y_min) / 2);

    // Add num randomly-chosen circles that would fit within those bounds
    std::vector<dcircle> circles;
    for (int i = 0; i < numCircles; ++i) {
        float x = dis_x(gen);
        float y = dis_y(gen);
        float r = dis_r(gen);
        circles.push_back(std::make_tuple(x, y, r));
    }

    return circles;
}

static std::vector<dcircle> makeInitialGuessNormal(int numCircles, dbounds b) {
    std::vector<dcircle> circles;
    float radius = std::min(b.x_max - b.x_min, b.y_max - b.y_min) / (20 * std::sqrt(numCircles));
    int grid_size = static_cast<int>(std::sqrt(numCircles));
    float x_step = (b.x_max - b.x_min) / grid_size;
    float y_step = (b.y_max - b.y_min) / grid_size;

    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            if (circles.size() >= (unsigned)numCircles) break;
            float x = b.x_min + i * x_step + x_step / 2;
            float y = b.y_min + j * y_step + y_step / 2;
            circles.push_back(std::make_tuple(x, y, radius));
        }
    }

    return circles;
}

static void initializeDistances(const std::vector<dcircle> &circles, dpoint *pointArray, int numPoints) {
    for (int i = 0; i < numPoints; ++i) {
        double px = pointArray[i].x;
        double py = pointArray[i].y;

        double min_dist = std::numeric_limits<double>::max();
        int closest_circle = 0;

        for (unsigned j = 0; j < circles.size(); ++j) {
            double cx = std::get<0>(circles[j]);
            double cy = std::get<1>(circles[j]);
            double r = std::get<2>(circles[j]);

            double dist = std::abs(std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy)) - r);
            if (dist < min_dist) {
                min_dist = dist;
                closest_circle = j;
            }
        }
        pointArray[i].closest_circle = closest_circle;
    }
}

std::vector<dcircle> generateCircles(dpointlist &pointlist, int num) {
    std::cout << "making initial guess\n";
    Eigen::VectorXd initialGuess(3 * num);
    std::vector<dcircle> circles = makeInitialGuess(pointlist, num, "normal");
    for (int i = 0; i < num; ++i) {
        initialGuess(3 * i) = std::get<0>(circles[i]);
        initialGuess(3 * i + 1) = std::get<1>(circles[i]);
        initialGuess(3 * i + 2) = std::get<2>(circles[i]);
    }

    std::cout << "loading point array\n";
    dpoint *pointArray = loadPointArray(pointlist);
    initializeDistances(circles, pointArray, pointlist.size());

    std::cout << "creating optimizer object\n";
    CircleOptimization optimization(pointArray, pointlist.size());
    gdc::GradientDescent<double, CircleOptimization, gdc::WolfeBacktracking<double>> optimizer;
    optimizer.setObjective(optimization);

    std::cout << "setting optimizer parameters\n";
    optimizer.setMaxIterations(150); // Increased max iterations for better convergence
    optimizer.setMinGradientLength(1e-10); // Tighter gradient tolerance
    optimizer.setMinStepLength(1e-9); // Tighter step length tolerance
    optimizer.setMomentum(0.8); // Increased momentum to help with dependencies
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

    delete [] pointArray;

    return optimizedCircles;
}

double CircleOptimization::operator()(const Eigen::VectorXd &params, Eigen::VectorXd &grad) const {
    double loss = 0.0;
    grad.setZero(params.size());

    double px, py, cx, cy, r, dist, dcx, dcy, dr, normal;
    for (unsigned i = 0; i < numPoints; ++i) {
        px = pointArray[i].x;
        py = pointArray[i].y;
        int closest_circle = pointArray[i].closest_circle;
        cx = params(3 * closest_circle);
        cy = params(3 * closest_circle + 1);
        r = params(3 * closest_circle + 2);

        normal = 3.0 / (params.size() * (numPoints / 500.0));

        dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
        loss += std::abs(dist - r) * normal;

        dcx = ((px - cx) * (dist - r)) / (dist * std::abs(dist - r));
        dcy = ((py - cy) * (dist - r)) / (dist * std::abs(dist - r));
        dr = (r - dist) / std::abs(dist - r);

        grad(3 * closest_circle) += dcx * normal;
        grad(3 * closest_circle + 1) += dcy * normal;
        grad(3 * closest_circle + 2) += dr * normal;
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
