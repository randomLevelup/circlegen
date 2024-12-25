/**
 * @file cgio.cpp
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief input/output implementations for circlegen
 */

#include "cgio.h"
#include "tinyxml2.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <vector>
#include <tuple>
#include <cmath>
#include <random>

#include <matplot/matplot.h>

using namespace tinyxml2;

#define BEZ_RES 10
#define P_PI 3.14159265358979323846

void renderPoints(matplot::axes_handle &ax, dpointlist &points);

// static void plotLines(matplot::axes_handle ax, const std::vector<dline> &lines);
// static void plotQuads(matplot::axes_handle ax, const std::vector<dquad> &quads);
// static void plotCubics(matplot::axes_handle ax, const std::vector<dcubic>& cubics);
// static void plotArcs(matplot::axes_handle ax, const std::vector<darc>& arcs);

dpointlist samplePaths(pathbundle &pb, float res);
static dpointlist sample_line(dline &line, float res, std::mt19937 gen);
static dpointlist sample_quad(dquad &quad, float res, std::mt19937 gen);
static dpointlist sample_cubic(dcubic &cubic, float res, std::mt19937 gen);
static dpointlist sample_arc(darc &arc, float res, std::mt19937 gen);

pathbundle parseSVG(const char *filename);
static void parsePath(const char *d, pathbundle &bundle);

void renderPoints(matplot::axes_handle &ax, dpointlist &points) {
    ax->hold(true);

    // plotLines(ax, std::get<0>(pb));
    // plotQuads(ax, std::get<1>(pb));
    // plotCubics(ax, std::get<2>(pb));
    // plotArcs(ax, std::get<3>(pb));

    std::vector<double> x(points.size()), y(points.size());
    std::transform(points.begin(), points.end(), x.begin(), [](const auto& point) { return std::get<0>(point); });
    std::transform(points.begin(), points.end(), y.begin(), [](const auto& point) { return std::get<1>(point); });

    ax->scatter(x, y);

    ax->x_axis().visible(false);
    ax->y_axis().visible(false);
    ax->box(false);
}

dpointlist samplePaths(pathbundle &pb, float res) {
    std::random_device rd;
    std::mt19937 gen(rd());

    dpointlist points;

    for (auto &line : std::get<0>(pb)) {
        auto line_points = sample_line(line, res, gen);
        points.insert(points.end(), line_points.begin(), line_points.end());
    }

    for (auto &quad : std::get<1>(pb)) {
        auto quad_points = sample_quad(quad, res, gen);
        points.insert(points.end(), quad_points.begin(), quad_points.end());
    }

    for (auto &cubic : std::get<2>(pb)) {
        auto cubic_points = sample_cubic(cubic, res, gen);
        points.insert(points.end(), cubic_points.begin(), cubic_points.end());
    }

    for (auto &arc : std::get<3>(pb)) {
        auto arc_points = sample_arc(arc, res, gen);
        points.insert(points.end(), arc_points.begin(), arc_points.end());
    }

    return points;
}

static dpointlist sample_line(dline &line, float res, std::mt19937 gen) {
    float p[] = {std::get<0>(line), std::get<1>(line), std::get<2>(line), std::get<3>(line)};

    float length = std::sqrt(std::pow(p[2] - p[0], 2) + std::pow(p[3] - p[1], 2));
    int num_samples = (int)(length * res);

    dpointlist points;
    for (int i = 0; i < num_samples; ++i) {
        float t = udist(0.0, 1.0)(gen);
        float x = (1 - t) * p[0] + t * p[2];
        float y = (1 - t) * p[1] + t * p[3];
        points.push_back(std::make_tuple(x, y));
    }
    return points;
}

static dpointlist sample_quad(dquad &quad, float res, std::mt19937 gen) {
    float p[] = {std::get<0>(quad), std::get<1>(quad), std::get<2>(quad), std::get<3>(quad),
                 std::get<4>(quad), std::get<5>(quad)};
    
    float length = std::sqrt(std::pow(p[4] - p[0], 2) + std::pow(p[5] - p[1], 2));
    int num_samples = (int)(length * res);

    dpointlist points;
    for (int i = 0; i < num_samples; ++i) {
        float t = udist(0.0, 1.0)(gen);
        float x = std::pow(1 - t, 2) * p[0] + 2 * (1 - t) * t * p[2] + std::pow(t, 2) * p[4];
        float y = std::pow(1 - t, 2) * p[1] + 2 * (1 - t) * t * p[3] + std::pow(t, 2) * p[5];
        points.push_back(std::make_tuple(x, y));
    }
    return points;
}

static dpointlist sample_cubic(dcubic &cubic, float res, std::mt19937 gen) {
    float p[] = {std::get<0>(cubic), std::get<1>(cubic), std::get<2>(cubic), std::get<3>(cubic),
                 std::get<4>(cubic), std::get<5>(cubic), std::get<6>(cubic), std::get<7>(cubic)};

    float length = std::sqrt(std::pow(p[6] - p[0], 2) + std::pow(p[7] - p[1], 2));
    int num_samples = (int)(length * res);

    dpointlist points;
    for (int i = 0; i < num_samples; ++i) {
        float t = udist(0.0, 1.0)(gen);
        float x = std::pow(1 - t, 3) * p[0] + 3 * std::pow(1 - t, 2) * t * p[2] + 
                  3 * (1 - t) * std::pow(t, 2) * p[4] + std::pow(t, 3) * p[6];
        float y = std::pow(1 - t, 3) * p[1] + 3 * std::pow(1 - t, 2) * t * p[3] +
                  3 * (1 - t) * std::pow(t, 2) * p[5] + std::pow(t, 3) * p[7];
        points.push_back(std::make_tuple(x, y));
    }
    return points;
}

static dpointlist sample_arc(darc &arc, float res, std::mt19937 gen) {
    float p[] = {std::get<0>(arc), std::get<1>(arc), std::get<2>(arc), std::get<3>(arc),
                 std::get<4>(arc), std::get<7>(arc), std::get<8>(arc)};
    bool isLarge = std::get<5>(arc);
    bool isCCW = std::get<6>(arc);

    float phi = p[4] * P_PI / 180.0;
    float cos_phi = std::cos(phi);
    float sin_phi = std::sin(phi);

    float xc = ((p[0] + p[5]) / 2) - (p[2] / p[1]) * ((p[6] - p[1]) / 2) * sin_phi;
    float yc = ((p[1] + p[6]) / 2) + (p[2] / p[1]) * ((p[5] - p[0]) / 2) * cos_phi;

    float t1 = std::atan2((p[1] - yc) * cos_phi - (p[0] - xc) * sin_phi,
                          ((p[0] - xc) / p[2]) * cos_phi + ((p[1] - yc) / p[3]) * sin_phi);
    float t2 = std::atan2((p[6] - yc) * cos_phi - (p[5] - xc) * sin_phi,
                          ((p[5] - xc) / p[2]) * cos_phi + ((p[6] - yc) / p[3]) * sin_phi);
    
    if (isLarge && std::abs(t2 - t1) <= P_PI) {
        t2 += 2 * P_PI;
    }
    else if (!isLarge && std::abs(t2 - t1) > P_PI) {
        t2 -= 2 * P_PI;
    }

    if (isCCW) {
        float temp = t1;
        t1 = t2; t2 = temp;
    }

    float length = std::sqrt(std::pow(p[2], 2) + std::pow(p[3], 2));
    int num_samples = (int)(length * res);

    dpointlist points;
    for (int i = 0; i < num_samples; ++i) {
        float t = udist(t1, t2)(gen);
        float x = xc + p[2] * std::cos(t) * cos_phi - p[3] * std::sin(t) * sin_phi;
        float y = yc + p[2] * std::cos(t) * sin_phi + p[3] * std::sin(t) * cos_phi;
        points.push_back(std::make_tuple(x, y));
    }
    return points;
}

/* plotLines / plotQuads / plotCubics / plotArcs are not needed
static void plotLines(matplot::axes_handle ax, const std::vector<dline> &lines) {
    for (const auto& line : lines) {
        float x1 = std::get<0>(line);
        float y1 = std::get<1>(line);
        float x2 = std::get<2>(line);
        float y2 = std::get<3>(line);

        std::vector<float> x = {x1, x2};
        std::vector<float> y = {y1, y2};

        ax->plot(x, y, "k");
    }
}

static void plotQuads(matplot::axes_handle ax, const std::vector<dquad> &quads) {
    for (dquad quad : quads) {
        auto bezier_points = sample_quad(quad, BEZ_RES);
        std::vector<float> x(bezier_points.size()), y(bezier_points.size());
        std::transform(bezier_points.begin(), bezier_points.end(), x.begin(), [](const auto& point) { return std::get<0>(point); });
        std::transform(bezier_points.begin(), bezier_points.end(), y.begin(), [](const auto& point) { return std::get<1>(point); });

        ax->plot(x, y, "k");
    }
}

static void plotCubics(matplot::axes_handle ax, const std::vector<dcubic>& cubics) {
    for (dcubic cubic : cubics) {
        auto bezier_points = sample_cubic(cubic, BEZ_RES);
        std::vector<float> x(bezier_points.size()), y(bezier_points.size());
        std::transform(bezier_points.begin(), bezier_points.end(), x.begin(), [](const auto& point) { return std::get<0>(point); });
        std::transform(bezier_points.begin(), bezier_points.end(), y.begin(), [](const auto& point) { return std::get<1>(point); });

        ax->plot(x, y, "k");
    }
}

static void plotArcs(matplot::axes_handle ax, const std::vector<darc>& arcs) {
    for (darc arc : arcs) {
        auto arc_points = sample_arc(arc, BEZ_RES);
        std::vector<float> x(arc_points.size()), y(arc_points.size());
        std::transform(arc_points.begin(), arc_points.end(), x.begin(), [](const auto& point) { return std::get<0>(point); });
        std::transform(arc_points.begin(), arc_points.end(), y.begin(), [](const auto& point) { return std::get<1>(point); });
        
        ax->plot(x, y, "k");
    }
}
*/

pathbundle parseSVG(const char *filename) {
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename);
    pathbundle bundle;

    XMLElement *svg = doc.FirstChildElement("svg");
    for (XMLElement *el = svg->FirstChildElement(); el; el = el->NextSiblingElement()) {
        std::string type = el->Name();
        if (type == "path") {
            const char *data = el->Attribute("d");
            parsePath(data, bundle);
        }
    }

    return bundle;
}

static void parsePath(const char *d, pathbundle &bundle) {
    std::stringstream dss(d);
            int pen[2] = {0, 0};
            char command;
            while (dss >> command) {
                std::vector<float> params;

                while (!isalpha(dss.peek()) && dss.peek() != EOF) {
                    std::string cparam;
                    char cc;
                    while (dss >> cc) {
                        cparam.push_back(cc);
                        
                        // if the next char is alpha or negative, break
                        if (isalpha(dss.peek()) || dss.peek() == '-') {
                            break;
                        }
                        // if next char is a separator, ignore it
                        if (dss.peek() == ' ' || dss.peek() == ',') {
                            dss.ignore(1);
                            break;
                        }
                    }
                    params.push_back(std::stof(cparam));
                }
                
                // process command
                if (command == 'M') {
                    pen[0] = params[0];
                    pen[1] = params[1];
                }
                else if (command == 'L') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  params[0], params[1]));
                    pen[0] = params[0];
                    pen[1] = params[1];
                }
                else if (command == 'l') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  pen[0] + params[0], pen[1] + params[1]));
                    pen[0] += params[0];
                    pen[1] += params[1];
                }
                else if (command == 'H') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  params[0], pen[1]));
                    pen[0] = params[0];
                }
                else if (command == 'h') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  pen[0] + params[0], pen[1]));
                    pen[0] += params[0];
                }
                else if (command == 'V') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  pen[0], params[0]));
                    pen[1] = params[0];
                }
                else if (command == 'v') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  pen[0], pen[1] + params[0]));
                    pen[1] += params[0];
                }
                else if (command == 'Q') {
                    std::get<1>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  params[0], params[1],
                                                                  params[2], params[3]));
                    pen[0] = params[2];
                    pen[1] = params[3];
                }
                else if (command == 'q') {
                    std::get<1>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  pen[0] + params[0], pen[1] + params[1],
                                                                  pen[0] + params[2], pen[1] + params[3]));
                }
                else if (command == 'C') {
                    std::get<2>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  params[0], params[1],
                                                                  params[2], params[3],
                                                                  params[4], params[5]));
                    pen[0] = params[4];
                    pen[1] = params[5];
                }
                else if (command == 'c') {
                    std::get<2>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  pen[0] + params[0], pen[1] + params[1],
                                                                  pen[0] + params[2], pen[1] + params[3],
                                                                  pen[0] + params[4], pen[1] + params[5]));
                    pen[0] += params[4];
                    pen[1] += params[5];
                }
                else if (command == 'A') {
                    std::get<3>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  params[0], params[1],
                                                                  params[2], params[3], params[4],
                                                                  params[5], params[6]));
                    pen[0] = params[5];
                    pen[1] = params[6];
                }
                else if (command == 'a') {
                    std::get<3>(bundle).push_back(std::make_tuple(pen[0], pen[1],
                                                                  pen[0] + params[0], pen[1] + params[1],
                                                                  params[2], params[3], params[4],
                                                                  params[5], params[6]));
                    pen[0] += params[5];
                    pen[1] += params[6];
                }
            }
}

