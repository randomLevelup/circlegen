/**
 * @file cgproc.cpp
 * @author Jupiter Westbard
 * @date 03/20/2025
 * @brief circlegen processing implementations
 */

#include <iostream>
#include <vector>
#include <tuple>
#include <cmath>
#include <random>
#include <algorithm>

#include <Eigen/Core>
#include "gdcpp.h"

#include "wasm_circlegen.h"

static void set_pixel(uint8_t *pixel, uint8_t val);
static double mag_factor(uint8_t *pixel);
dpixmap sobelFilter(dpixmap pm);
dpointlist samplePoints(dpixmap pm, int num, double threshold);
dpointlist trimPointlist(dpointlist &pointlist, const dcircle &circle, int threshold);
std::vector<dcircle> generateCircles(dpointlist &pointlist, dpixmap *pm, int num);

static void set_pixel(uint8_t *pixel, uint8_t val) {
    *pixel = val;
    *(pixel + 1) = val;
    *(pixel + 2) = val;
}

static double mag_factor(uint8_t *pixel) {
    int r = *pixel;
    int g = *(pixel + 1);
    int b = *(pixel + 2);
    double mag = sqrt(r * r + g * g + b * b);
    return (255.0 - mag) / 255.0;
}

struct CircleOptimization {
    static dpixmap *dpm;
    static dpointlist dpl;
    static dcircle last;

    CircleOptimization() { 
        last = std::make_tuple(0.0, 0.0, 0.0);
    }

    static void initialize(dpointlist &points, dpixmap *pixmap) {
        dpm = pixmap;
        dpl = points;
    }

    double operator()(const Eigen::VectorXd &params, Eigen::VectorXd &) const {
        double total_loss = 0.0;
        double cx = params(0);
        double cy = params(1);
        double r = params(2);

        int count = 0;
        for (const auto &point : dpl) {
            double x = std::get<0>(point);
            double y = std::get<1>(point);
            double dist_center = std::sqrt((cx - x) * (cx - x) + (cy - y) * (cy - y));
            double dist_edge = std::abs(dist_center - r);
            if (dist_edge < 150) {
                ++count;
                total_loss += dist_edge;
            }
        }

        return total_loss / (double)count;
    }
};

dpixmap* CircleOptimization::dpm = nullptr;
dpointlist CircleOptimization::dpl;
dcircle CircleOptimization::last = std::make_tuple(0.0, 0.0, 0.0);

dpixmap sobelFilter(dpixmap pm) {
    dpixmap filtered = {pm.width, pm.height, new uint8_t[pm.width * pm.height * 3]};
    int width = pm.width;
    int height = pm.height;

    // Sobel kernels
    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int Gy[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int sumX = 0;
            int sumY = 0;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixelValue = pm.data[((y + ky) * width + (x + kx)) * 3];
                    sumX += pixelValue * Gx[ky + 1][kx + 1];
                    sumY += pixelValue * Gy[ky + 1][kx + 1];
                }
            }

            uint8_t magnitude = static_cast<uint8_t>(std::sqrt(sumX * sumX + sumY * sumY));
            if (magnitude > 255) magnitude = 255;

            set_pixel(&filtered.data[(y * width + x) * 3], magnitude);
        }
    }

    return filtered;
}

dpointlist samplePoints(dpixmap pm, size_t num, double threshold) {
    dpointlist points;

    for (int i = 0; i < (pm.width * pm.height); ++i) {
        double mag = mag_factor(pm.data + (i * 3));
        if (mag < threshold) {
            int x = i % pm.width;
            int y = i / pm.width;
            points.push_back(std::make_tuple(x, y));
        }
    }

    // shuffle the points
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(points.begin(), points.end(), g);

    // pick up to num points
    if (points.size() > num) {
        points.resize(num);
    }
    return points;
}

dpointlist trimPointlist(dpointlist &pointlist, const dcircle &circle, int threshold) {
    dpointlist trimmed;
    double cx = std::get<0>(circle);
    double cy = std::get<1>(circle);
    double r = std::get<2>(circle);
    
    for (const auto &point : pointlist) {
        double x = std::get<0>(point);
        double y = std::get<1>(point);
        double dist_center = std::sqrt((cx - x) * (cx - x) + (cy - y) * (cy - y));
        double dist_edge = std::abs(dist_center - r);
        if (dist_edge > threshold) {
            trimmed.push_back(point);
        }
    }
    
    return trimmed;
}

static gdc::GradientDescent<double, CircleOptimization,
    gdc::BarzilaiBorwein<double>> makeOptimizer() {

    gdc::GradientDescent<double, CircleOptimization,
        gdc::BarzilaiBorwein<double>> optimizer;
    
    optimizer.setMaxIterations(40);
    optimizer.setMinGradientLength(0.06);
    optimizer.setMinStepLength(1e-9);
    optimizer.setVerbosity(0);

    return optimizer;
}

std::vector<dcircle> generateCircles(dpointlist &pointlist, dpixmap *pm, int num) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis;

    std::vector<dcircle> circles;

    int fail_count = 0;
    while (true) {
        if (circles.size() >= (unsigned)num) {
            return circles;
        }
        if (pointlist.size() <= 3 || fail_count > 100) {
            // no more points or too many failures: exit optimization loop
            break;
        }
        // pick 2 random points from pointlist
        dis = std::uniform_int_distribution<int>(0, pointlist.size() - 1);
        dpoint p1 = pointlist[dis(gen)];
        dpoint p2 = pointlist[dis(gen)];

        // p1 == center, p2 == edge
        double cx = std::get<0>(p1);
        double cy = std::get<1>(p1);
        double r = std::sqrt((std::get<0>(p1) - std::get<0>(p2)) * (std::get<0>(p1) - std::get<0>(p2)) +
                             (std::get<1>(p1) - std::get<1>(p2)) * (std::get<1>(p1) - std::get<1>(p2)));
        
        Eigen::VectorXd initialGuess(3);
        initialGuess(0) = cx;
        initialGuess(1) = cy;
        initialGuess(2) = r;

        auto opt = makeOptimizer();
        CircleOptimization::initialize(pointlist, pm);
        CircleOptimization circleOpt;
        opt.setObjective(circleOpt);

        auto result = opt.minimize(initialGuess);

        if (result.fval && result.fval > 0) {
            circles.push_back(std::make_tuple(result.xval(0), result.xval(1), result.xval(2)));
            dcircle &new_circle = circles.back();
            pointlist = trimPointlist(pointlist, new_circle, 20);
            fail_count = 0;
        }
        else { ++fail_count; }
    }
    // fallback: place random circles until reaching requested number
    std::uniform_int_distribution<int> randx(0, pm->width - 1);
    std::uniform_int_distribution<int> randy(0, pm->height - 1);
    double maxR = std::min(pm->width, pm->height) / 2.0;
    std::uniform_real_distribution<double> randr(1.0, maxR);
    while (circles.size() < (unsigned)num) {
        double cx = randx(gen);
        double cy = randy(gen);
        double r = randr(gen);
        circles.push_back(std::make_tuple(cx, cy, r));
    }
    return circles;
}
