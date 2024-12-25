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
 * @brief generate and optimize circles
 * @param points point list to fit circles to
 * @param num number of circles
 * @return vector of n circles
 */
std::vector<dcircle> generateCircles(dpointlist &points, int num);

/**
 * @brief spawns n circles with random dimensions
 * @param points points to determine bounds
 * @param num number of circles
 * @return vector of n circles
 */
std::vector<dcircle> makeInitialGuess(dpointlist &points, int num);
