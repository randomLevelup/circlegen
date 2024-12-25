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

std::vector<dcircle> initialGuess(dpointlist &points, int num);
