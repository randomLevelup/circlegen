/**
 * @file cgproc.cpp
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief circlegen processing implementations
 */

#include <vector>
#include <algorithm>
#include <random>
#include <tuple>

#include "cgproc.h"
#include "cgio.h"

std::vector<dcircle> initialGuess(dpointlist &points, int num);

std::vector<dcircle> initialGuess(dpointlist &points, int num) {
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
