/**
 * @file cgio.h
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief input/output function headers for circlegen
 */

#include <tuple>
#include <vector>
#include <random>

#pragma once

typedef std::uniform_real_distribution<float> udist;

typedef std::vector<std::tuple<float, float>> dpointlist;

typedef std::tuple<float, float, float, float> dline;
typedef std::tuple<float, float, float, float, float, float> dquad;
typedef std::tuple<float, float, float, float, float, float, float, float> dcubic;
typedef std::tuple<float, float, float, float, float, bool, bool, float, float> darc;

typedef std::tuple<std::vector<dline>,
                   std::vector<dquad>,
                   std::vector<dcubic>,
                   std::vector<darc>> pathbundle;

typedef std::tuple<float, float, float> dcircle;
typedef std::tuple<std::vector<dcircle>, dpointlist> dbundle;

/**
 * @brief 
 * @param filename 
 * @return pathbundle with lists of lines, quads, cubics and arcs in absolute position
 */
pathbundle parseSVG(const char *filename);

/**
 * @brief sample points along all paths
 * @param pb pathbundle containing lines/quads/cubics/arcs
 * @param res sampling resolution (relative to overall xy range)
 * @return dpointlist of sampled points
 */
dpointlist samplePaths(pathbundle &pb, float res);

/**
 * @brief render circles and points and save as "output.jpg"
 * @param bundle bundle with a vector of dcircles and a dpointlist
 * @param scaleFactor integer resolution scalar
 * @param app QApplication instance
 */
void renderImage(const dbundle &bundle, const int scaleFactor);
