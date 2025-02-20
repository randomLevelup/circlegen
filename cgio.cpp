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

using namespace tinyxml2;

#define BEZ_RES 10
#define P_PI 3.14159265358979323846

static dpointlist sample_circle(dcircle &circle, int num_samples);
dpointlist samplePaths(pathbundle &pb, float res);

static dpointlist sample_line(dline &line, float res, std::mt19937 gen);
static dpointlist sample_quad(dquad &quad, float res, std::mt19937 gen);
static dpointlist sample_cubic(dcubic &cubic, float res, std::mt19937 gen);
static dpointlist sample_arc(darc &arc, float res, std::mt19937 gen);

pathbundle parseSVG(const char *filename);
static void parseSVGElement(XMLElement *element, pathbundle &bundle);
static void parsePath(const char *d, pathbundle &bundle);
static void parsePolygon(const char *p, pathbundle &bundle);

static dpointlist sample_circle(dcircle &circle, int num_samples) {
    float x = std::get<0>(circle);
    float y = std::get<1>(circle);
    float r = std::get<2>(circle);

    dpointlist points;
    for (int i = 0; i < num_samples; ++i) {
        float theta = 2 * P_PI * i / num_samples;
        float px = x + r * std::cos(theta);
        float py = y + r * std::sin(theta);
        points.push_back(std::make_tuple(px, py));
    }
    return points;
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

    float dx2 = (p[0] - p[5]) / 2.0;
    float dy2 = (p[1] - p[6]) / 2.0;
    float x1 = cos_phi * dx2 + sin_phi * dy2;
    float y1 = -sin_phi * dx2 + cos_phi * dy2;

    float rx_sq = p[2] * p[2];
    float ry_sq = p[3] * p[3];
    float x1_sq = x1 * x1;
    float y1_sq = y1 * y1;

    float radicant = (rx_sq * ry_sq - rx_sq * y1_sq - ry_sq * x1_sq) / (rx_sq * y1_sq + ry_sq * x1_sq);
    radicant = (radicant < 0) ? 0 : radicant;
    float coef = (isLarge != isCCW ? 1 : -1) * std::sqrt(radicant);
    float cx1 = coef * ((p[2] * y1) / p[3]);
    float cy1 = coef * -((p[3] * x1) / p[2]);

    float cx = cos_phi * cx1 - sin_phi * cy1 + (p[0] + p[5]) / 2.0;
    float cy = sin_phi * cx1 + cos_phi * cy1 + (p[1] + p[6]) / 2.0;

    float start_angle = std::atan2((y1 - cy1) / p[3], (x1 - cx1) / p[2]);
    float end_angle = std::atan2((-y1 - cy1) / p[3], (-x1 - cx1) / p[2]);

    if (!isCCW && end_angle > start_angle) {
        end_angle -= 2 * P_PI;
    } else if (isCCW && end_angle < start_angle) {
        end_angle += 2 * P_PI;
    }

    float length = std::abs(end_angle - start_angle) * std::sqrt(rx_sq + ry_sq) / 2.0;
    int num_samples = (int)(length * res);

    dpointlist points;
    for (int i = 0; i < num_samples; ++i) {
        float t = udist(start_angle, end_angle)(gen);
        float x = cx + p[2] * std::cos(t) * cos_phi - p[3] * std::sin(t) * sin_phi;
        float y = cy + p[2] * std::cos(t) * sin_phi + p[3] * std::sin(t) * cos_phi;
        points.push_back(std::make_tuple(x, y));
    }
    return points;
}

pathbundle parseSVG(const char *filename) {
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename);
    pathbundle bundle;

    XMLElement *svg = doc.FirstChildElement("svg");
    if (svg) {
        const char *viewBoxStr = svg->Attribute("viewBox");

        if (viewBoxStr) {
            std::stringstream ss(viewBoxStr);
            float minX, minY, width, height;
            ss >> minX >> minY >> width >> height;
            std::get<4>(bundle) = width;
            std::get<5>(bundle) = height;
            std::get<6>(bundle) = minX;
            std::get<7>(bundle) = minY;
        } else {
            std::cerr << "SVG viewBox not specified" << std::endl;
            const char *widthStr = svg->Attribute("width");
            const char *heightStr = svg->Attribute("height");
            if (widthStr && heightStr) {
                std::get<4>(bundle) = std::stof(widthStr);
                std::get<5>(bundle) = std::stof(heightStr);
                std::get<6>(bundle) = 0;
                std::get<7>(bundle) = 0;
            } else {
                std::cerr << "Error: could not read width and height in " << filename << std::endl;
            }
        }

        auto gtag = svg->FirstChildElement("g");
        if (gtag) {
            const char *transform = gtag->Attribute("transform");
            if (transform) {
                std::string tstr(transform);
                
                // Check for scale
                std::string::size_type scaleStart = tstr.find("scale(");
                if (scaleStart != std::string::npos) {
                    scaleStart += 6;
                    std::string::size_type scaleEnd = tstr.find(")", scaleStart);
                    if (scaleEnd != std::string::npos) {
                        std::string scale = tstr.substr(scaleStart, scaleEnd - scaleStart);
                        float scalefactor = std::stof(scale);
                        std::get<4>(bundle) *= scalefactor;
                        std::get<5>(bundle) *= scalefactor;
                    }
                }

                // Check for translate
                std::string::size_type translateStart = tstr.find("translate(");
                if (translateStart != std::string::npos) {
                    translateStart += 10;
                    std::string::size_type translateEnd = tstr.find(")", translateStart);
                    if (translateEnd != std::string::npos) {
                        std::string translate = tstr.substr(translateStart, translateEnd - translateStart);
                        std::stringstream ss(translate);
                        float tx, ty;
                        char comma;
                        if (ss >> tx >> comma >> ty) {
                            std::get<6>(bundle) += tx;
                            std::get<7>(bundle) += ty;
                        }
                    }
                }
            }
        }

        parseSVGElement(svg->FirstChildElement(), bundle);
    } else {
        std::cerr << "Error: could not read svg element in " << filename << std::endl;
    }

    return bundle;
}

static void parseSVGElement(XMLElement *element, pathbundle &bundle) {
    if (!element) return;

    std::string type = element->Name();
    if (type == "path") {
        const char *data = element->Attribute("d");
        if (data) {
            parsePath(data, bundle);
        }
    }
    else if (type == "polygon") {
        const char *points = element->Attribute("points");
        if (points) {
            parsePolygon(points, bundle);
        }
    }

    // Recurse on any child elements
    parseSVGElement(element->FirstChildElement(), bundle);
    // Recurse on next sibling
    parseSVGElement(element->NextSiblingElement(), bundle);
}

static void parsePath(const char *d, pathbundle &bundle) {
    const float width = std::get<4>(bundle);
    const float height = std::get<5>(bundle);

    std::stringstream dss(d);
    float pen[2] = {0, 0};
    char command;
    char last_command = '\0';
    while (dss >> command) {
        if (isalpha(command)) {
            last_command = command;
        } else {
            dss.putback(command);
            command = last_command;
        }
        std::vector<float> params;

        while (!isalpha(dss.peek()) && dss.peek() != EOF) {
            std::string cparam;
            char cc;
            while (dss >> cc) {
                if (isalpha(cc)) {
                    dss.putback(cc);
                    break;
                }
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
            if (!cparam.empty()) {
                params.push_back(std::stof(cparam));
            }
        }

        // Convert coordinates to percentages, excluding angle parameters and flags
        if (command == 'A' || command == 'a') {
            for (size_t i = 0; i < params.size(); i += 7) {
                params[i] /= width;       // rx
                params[i + 1] /= height;  // ry
                // params[i + 2] is the angle and should not be converted
                // params[i + 3] and params[i + 4] are flags and should not be converted
                params[i + 5] /= width;   // x
                params[i + 6] /= height;  // y
            }
        } else {
            for (size_t i = 0; i < params.size(); i += 2) {
                params[i] /= width;
                if (i + 1 < params.size()) {
                    params[i + 1] /= height;
                }
            }
        }
        
        // process command
        if (command == 'M' || command == 'm') {
            for (size_t i = 0; i < params.size(); i += 2) {
                if (command == 'M') {
                    pen[0] = params[i];
                    pen[1] = params[i + 1];
                } else {
                    pen[0] += params[i];
                    pen[1] += params[i + 1];
                }
                if (i == 0) continue; // MoveTo command does not create a line
                std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1], params[i], params[i + 1]));
            }
        }
        else if (command == 'L' || command == 'l') {
            for (size_t i = 0; i < params.size(); i += 2) {
                if (command == 'L') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1], params[i], params[i + 1]));
                    pen[0] = params[i];
                    pen[1] = params[i + 1];
                } else {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1], pen[0] + params[i], pen[1] + params[i + 1]));
                    pen[0] += params[i];
                    pen[1] += params[i + 1];
                }
            }
        }
        else if (command == 'H' || command == 'h') {
            for (size_t i = 0; i < params.size(); ++i) {
                if (command == 'H') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1], params[i], pen[1]));
                    pen[0] = params[i];
                } else {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1], pen[0] + params[i], pen[1]));
                    pen[0] += params[i];
                }
            }
        }
        else if (command == 'V' || command == 'v') {
            for (size_t i = 0; i < params.size(); ++i) {
                if (command == 'V') {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1], pen[0], params[i]));
                    pen[1] = params[i];
                } else {
                    std::get<0>(bundle).push_back(std::make_tuple(pen[0], pen[1], pen[0], pen[1] + params[i]));
                    pen[1] += params[i];
                }
            }
        }
        else if (command == 'Q' || command == 'q') {
            for (size_t i = 0; i < params.size(); i += 4) {
                if (command == 'Q') {
                    std::get<1>(bundle).push_back(std::make_tuple(pen[0], pen[1], params[i], params[i + 1], params[i + 2], params[i + 3]));
                    pen[0] = params[i + 2];
                    pen[1] = params[i + 3];
                } else {
                    std::get<1>(bundle).push_back(std::make_tuple(pen[0], pen[1], pen[0] + params[i], pen[1] + params[i + 1], pen[0] + params[i + 2], pen[1] + params[i + 3]));
                    pen[0] += params[i + 2];
                    pen[1] += params[i + 3];
                }
            }
        }
        else if (command == 'C' || command == 'c') {
            for (size_t i = 0; i < params.size(); i += 6) {
                if (command == 'C') {
                    std::get<2>(bundle).push_back(std::make_tuple(pen[0], pen[1], params[i], params[i + 1], params[i + 2], params[i + 3], params[i + 4], params[i + 5]));
                    pen[0] = params[i + 4];
                    pen[1] = params[i + 5];
                } else {
                    std::get<2>(bundle).push_back(std::make_tuple(pen[0], pen[1], pen[0] + params[i], pen[1] + params[i + 1], pen[0] + params[i + 2], pen[1] + params[i + 3], pen[0] + params[i + 4], pen[1] + params[i + 5]));
                    pen[0] += params[i + 4];
                    pen[1] += params[i + 5];
                }
            }
        }
        else if (command == 'A' || command == 'a') {
            for (size_t i = 0; i < params.size(); i += 7) {
                if (command == 'A') {
                    std::get<3>(bundle).push_back(std::make_tuple(pen[0], pen[1], params[i], params[i + 1], params[i + 2], params[i + 3], params[i + 4], params[i + 5], params[i + 6]));
                    pen[0] = params[i + 5];
                    pen[1] = params[i + 6];
                } else {
                    std::get<3>(bundle).push_back(std::make_tuple(pen[0], pen[1], params[i], params[i + 1], params[i + 2], params[i + 3], params[i + 4], pen[0] + params[i + 5], pen[1] + params[i + 6]));
                    pen[0] += params[i + 5];
                    pen[1] += params[i + 6];
                }
            }
        }
    }
}

static void parsePolygon(const char *p, pathbundle &bundle) {
    const float width = std::get<4>(bundle);
    const float height = std::get<5>(bundle);

    std::stringstream pss(p);
    std::vector<float> params;
    float value;
    char comma;

    while (pss >> value) {
        params.push_back(value);
        if (pss.peek() == ',') {
            pss >> comma;
        }
    }

    for (size_t i = 0; i < params.size(); i += 2) {
        params[i] /= width;
        if (i + 1 < params.size()) {
            params[i + 1] /= height;
        }
    }

    for (size_t i = 0; i < params.size(); i += 2) {
        if (i == 0) {
            std::get<0>(bundle).push_back(std::make_tuple(params[i], params[i + 1], params[i], params[i + 1]));
        } else {
            std::get<0>(bundle).push_back(std::make_tuple(params[i - 2], params[i - 1], params[i], params[i + 1]));
        }
    }

    if (params.size() >= 4) {
        std::get<0>(bundle).push_back(std::make_tuple(params[params.size() - 2], params[params.size() - 1], params[0], params[1]));
    }
}
