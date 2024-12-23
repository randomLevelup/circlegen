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

#include <matplot/matplot.h>

using namespace tinyxml2;

#define BEZ_RES 10
#define P_PI 3.14159265358979323846

void renderPoints(matplot::axis_type ax, dpointlist &points, const char *filename);

static void plotLines(matplot::axes_handle ax, const std::vector<dline> &lines);
static void plotQuads(matplot::axes_handle ax, const std::vector<dquad> &quads);
static void plotCubics(matplot::axes_handle ax, const std::vector<dcubic>& cubics);
static void plotArcs(matplot::axes_handle ax, const std::vector<darc>& arcs);

dpointlist samplePaths(pathbundle &pb, int res);

pathbundle parseSVG(const char *filename);
static void parsePath(const char *d, pathbundle &bundle);

void renderPoints(matplot::axes_handle &ax, dpointlist &points) {
    ax->hold(true);

    // plotLines(ax, std::get<0>(pb));
    // plotQuads(ax, std::get<1>(pb));
    // plotCubics(ax, std::get<2>(pb));
    // plotArcs(ax, std::get<3>(pb));

    ax->x_axis().visible(false);
    ax->y_axis().visible(false);
    ax->box(false);
}

dpointlist samplePaths(pathbundle &pb, int res) {
    dpointlist points;

    for (auto &dline : std::get<0>(pb)) {
        auto line_points = sample_line(dline, res);
        points.insert(points.end(), line_points.begin(), line_points.end());
    }

    for (auto &dquad : std::get<1>(pb)) {
        auto quad_points = sample_quad(dquad, res);
        points.insert(points.end(), quad_points.begin(), quad_points.end());
    }

    for (auto &dcubic : std::get<2>(pb)) {
        auto cubic_points = sample_cubic(dcubic, res);
        points.insert(points.end(), cubic_points.begin(), cubic_points.end());
    }

    for (auto &darc : std::get<3>(pb)) {
        auto arc_points = sample_arc(darc, res);
        points.insert(points.end(), arc_points.begin(), arc_points.end());
    }

    return points;
}

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
    for (const auto& quad : quads) {
        auto [x0, y0, x1, y1, x2, y2] = quad;
        auto bezier_points = sample_quad(x0, y0, x1, y1, x2, y2, BEZ_RES);
        std::vector<float> x(bezier_points.size()), y(bezier_points.size());
        std::transform(bezier_points.begin(), bezier_points.end(), x.begin(), [](const auto& point) { return std::get<0>(point); });
        std::transform(bezier_points.begin(), bezier_points.end(), y.begin(), [](const auto& point) { return std::get<1>(point); });

        ax->plot(x, y, "k");
    }
}

static void plotCubics(matplot::axes_handle ax, const std::vector<dcubic>& cubics) {
    for (const auto& cubic : cubics) {
        auto [x0, y0, x1, y1, x2, y2, x3, y3] = cubic;
        auto bezier_points = sample_cubic(x0, y0, x1, y1, x2, y2, x3, y3, BEZ_RES);
        std::vector<float> x(bezier_points.size()), y(bezier_points.size());
        std::transform(bezier_points.begin(), bezier_points.end(), x.begin(), [](const auto& point) { return std::get<0>(point); });
        std::transform(bezier_points.begin(), bezier_points.end(), y.begin(), [](const auto& point) { return std::get<1>(point); });

        ax->plot(x, y, "k");
    }
}

static void plotArcs(matplot::axes_handle ax, const std::vector<darc>& arcs) {
    for (const auto& arc : arcs) {
        auto arc_points = sample_arc
        (
            std::get<0>(arc), std::get<1>(arc),
            std::get<2>(arc), std::get<3>(arc),
            std::get<4>(arc), std::get<5>(arc),
            std::get<6>(arc), std::get<7>(arc),
            std::get<8>(arc), BEZ_RES
        );
        std::vector<float> x(arc_points.size()), y(arc_points.size());
        std::transform(arc_points.begin(), arc_points.end(), x.begin(), [](const auto& point) { return std::get<0>(point); });
        std::transform(arc_points.begin(), arc_points.end(), y.begin(), [](const auto& point) { return std::get<1>(point); });
        
        ax->plot(x, y, "k");
    }
}


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

