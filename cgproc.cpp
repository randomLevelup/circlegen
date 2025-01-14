/**
 * @file cgproc.cpp
 * @author Jupiter Westbard
 * @date 12/19/2024
 * @brief circlegen processing implementations
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <tuple>
#include <limits>
#include <cassert>
#include <algorithm>

#include <Eigen/Core>
#include "gdcpp.h"

#include <cairo.h>
#include <librsvg/rsvg.h>

#include "cgproc.h"
#include "cgio.h"

static dpoint *pointListToArray(const dpointlist &pointlist);
static dpointlist pointArrayToList(const dpoint *pointArray, unsigned num, unsigned start);
static unsigned trimPointArray(dpoint *pointArray, unsigned num, unsigned start, const Eigen::VectorXd &circle);
static Eigen::VectorXd spawnCircle(dpoint *pointArray, unsigned numPoints, unsigned startIndex);
std::tuple<std::vector<dcircle>, dpointlist> generateCircles(dpointlist &pointlist, int num);
PixelStream getSVGStream(const char *filename);

static dpoint *pointListToArray(const dpointlist &pointlist) {
    dpoint *pointarray = new dpoint[pointlist.size()];
    for (unsigned i = 0; i < pointlist.size(); ++i) {
        pointarray[i].x = std::get<0>(pointlist[i]);
        pointarray[i].y = std::get<1>(pointlist[i]);
    }
    return pointarray;
}

static dpointlist pointArrayToList(const dpoint *pointArray, unsigned num, unsigned start) {
    dpointlist pointlist;
    for (unsigned i = start; i < num; ++i) {
        pointlist.push_back(std::make_tuple(pointArray[i].x, pointArray[i].y));
    }
    return pointlist;
}

static unsigned trimPointArray(dpoint *pointArray, unsigned num, unsigned start, const Eigen::VectorXd &circle) {
    double cx, cy, r, dist;
    cx = circle(0);
    cy = circle(1);
    r = circle(2);

    int deleted = 0;
    for (unsigned i = start; i < num; ++i) {
        dist = std::sqrt((pointArray[i].x - cx) * (pointArray[i].x - cx) +
                         (pointArray[i].y - cy) * (pointArray[i].y - cy));
        if (std::abs(dist - r) < 2.5) {
            std::swap(pointArray[i], pointArray[start]);
            start++;
            deleted++;
        }
    }
    printf("deleted %d points\n", deleted);
    return start;
}

static Eigen::VectorXd spawnCircle(dpoint *pointArray, unsigned numPoints, unsigned startIndex) {
    std::cout << "numPoints: " << numPoints << ", startIndex: " << startIndex << std::endl;
    assert(startIndex < numPoints);
    std::random_device rd;
    std::mt19937 gen(rd());
    // pick 2 random points from pointArray between startIndex and numPoints
    std::uniform_int_distribution<int> dis(startIndex, numPoints - 1);
    dpoint p1 = pointArray[dis(gen)];
    dpoint p2 = pointArray[dis(gen)];

    Eigen::VectorXd params(3);
    params(0) = p1.x;
    params(1) = p1.y;
    params(2) = std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));

    return params;
}

std::tuple<std::vector<dcircle>, dpointlist> generateCircles(dpointlist &pointlist, int num) {
    std::cout << "creating optimizer\n";
    dpoint *pointArray = pointListToArray(pointlist);
    CircleOptimization opt(pointArray, pointlist.size());
    gdc::GradientDescent<double, CircleOptimization, gdc::WolfeBacktracking<double>> optimizer;
    optimizer.setObjective(opt);
    optimizer.setMaxIterations(250);
    optimizer.setMinGradientLength(1e-9);
    optimizer.setMinStepLength(1e-9);
    optimizer.setMomentum(0.9);
    optimizer.setVerbosity(0);

    std::cout << "generating circles\n";
    std::vector<dcircle> circles;
    while (opt.startIndex < opt.numPoints && circles.size() < (unsigned)num) {
        Eigen::VectorXd params = spawnCircle(pointArray, opt.numPoints, opt.startIndex);
        // circles.push_back(std::make_tuple(params(0), params(1), params(2)));

        // optimize params
        auto result = optimizer.minimize(params);
        std::cout << "converged: " << (result.converged ? "true" : "false") << std::endl;
        std::cout << "iterations: " << result.iterations << std::endl;
        std::cout << "\nnew circle with fval [" << result.fval << "]\n\n";

        // update return vector
        circles.push_back(std::make_tuple(result.xval(0), result.xval(1), result.xval(2)));

        // update pointArray
        opt.startIndex = trimPointArray(pointArray, opt.numPoints, opt.startIndex, result.xval);
    }
    std::tuple<std::vector<dcircle>, dpointlist> result = std::make_tuple(
        circles,
        pointArrayToList(pointArray, opt.numPoints, opt.startIndex)
    );
    delete[] pointArray;
    return result;
}

double CircleOptimization::operator()(const Eigen::VectorXd &params, Eigen::VectorXd &) const {
    double total_loss = 0.0;

    double cx, cy, r, dist, loss;
    cx = params(0);
    cy = params(1);
    r = params(2);

    for (unsigned i = startIndex; i < numPoints; ++i) {
        dist = std::sqrt((pointArray[i].x - cx) * (pointArray[i].x - cx) +
                         (pointArray[i].y - cy) * (pointArray[i].y - cy));
        loss = std::sqrt(std::abs(dist - r));
        loss = (loss > 2.0) ? 1.0 : loss;
        total_loss += loss;
    }
    total_loss /= (double)numPoints;
    return total_loss * 100;
}

PixelStream getSVGStream(const char *filename) {
    PixelStream res = PixelStream();

    RsvgHandle *handle = rsvg_handle_new_from_file(filename, NULL);
    if (!handle) {
        std::cerr << "Error: Unable to create RsvgHandle from file " << filename << std::endl;
        return res;
    }

    RsvgDimensionData dimensions;
    gdouble width, height;
    rsvg_handle_get_intrinsic_size_in_pixels(handle, &width, &height);
    dimensions.width = static_cast<int>(width);
    dimensions.height = static_cast<int>(height);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dimensions.width, dimensions.height);
    cairo_t *cr = cairo_create(surface);

    RsvgRectangle viewport = {0, 0, static_cast<double>(dimensions.width), static_cast<double>(dimensions.height)};
    rsvg_handle_render_document(handle, cr, &viewport, NULL);

    unsigned char *data = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);

    for (int y = 0; y < dimensions.height; ++y) {
        for (int x = 0; x < dimensions.width; ++x) {
            unsigned char *pixel = data + y * stride + x * 4;
            res << pixel[2] << pixel[1] << pixel[0];
        }
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);

    return res;
}