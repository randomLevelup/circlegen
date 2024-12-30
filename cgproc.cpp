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
#include <cassert>

#include <Eigen/Core>
#include "gdcpp.h"

#include "cgproc.h"
#include "cgio.h"

static dpoint *loadPointArray(const dpointlist &pointlist);
static Eigen::VectorXd spawnCircle(dpoint *pointArray, int numPoints, int startIndex);
std::vector<dcircle> generateCircles(dpointlist &pointlist, int num);

static dpoint *loadPointArray(const dpointlist &pointlist) {
    dpoint *pointarray = new dpoint[pointlist.size()];
    for (unsigned i = 0; i < pointlist.size(); ++i) {
        pointarray[i].x = std::get<0>(pointlist[i]);
        pointarray[i].y = std::get<1>(pointlist[i]);
    }
    return pointarray;
}

static Eigen::VectorXd spawnCircle(dpoint *pointArray, int numPoints, int startIndex) {
    std::cout << "numPoints: " << numPoints << ", startIndex: " << startIndex << std::endl;
    assert(startIndex >= 0 && startIndex < numPoints);
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

std::vector<dcircle> generateCircles(dpointlist &pointlist, int num) {
    std::cout << "creating optimizer\n";
    dpoint *pointArray = loadPointArray(pointlist);
    CircleOptimization opt(pointArray, pointlist.size());
    gdc::GradientDescent<double, CircleOptimization, gdc::WolfeBacktracking<double>> optimizer;
    optimizer.setObjective(opt);
    optimizer.setMaxIterations(200); // Increased max iterations for better convergence
    optimizer.setMinGradientLength(1e-9); // Tighter gradient tolerance
    optimizer.setMinStepLength(1e-11); // Tighter step length tolerance
    optimizer.setMomentum(0.9); // Increased momentum to help with dependencies
    optimizer.setVerbosity(1);

    std::cout << "generating circles\n";
    std::vector<dcircle> circles;
    while (opt.startIndex < opt.numPoints && circles.size() < (unsigned)num) {
        Eigen::VectorXd params = spawnCircle(pointArray, opt.numPoints, opt.startIndex);

        // optimize params
        auto result = optimizer.minimize(params);
        std::cout << "converged: " << (result.converged ? "true" : "false") << std::endl;
        std::cout << "iterations: " << result.iterations << std::endl;
        std::cout << "\nnew circle with fval [" << result.fval << "]\n\n";

        // update return vector
        circles.push_back(std::make_tuple(result.xval(0), result.xval(1), result.xval(2)));

        // update pointArray
        opt.startIndex = opt.numPoints; // temp
    }

    delete[] pointArray;
    return circles;
}

double CircleOptimization::operator()(const Eigen::VectorXd &params, Eigen::VectorXd &grad) const {
    double total_loss = 0.0;
    grad.setZero(params.size());

    double a, b, c, r, cx, cy, px, py, dist2, distc, distnorm, loss, dl, dx, dy, dr;
    a = 10000.0;
    c = 0.9;

    for (unsigned i = startIndex; i < numPoints; ++i) {
        px = pointArray[i].x;
        py = pointArray[i].y;
        cx = params(0);
        cy = params(1);
        r = params(2);
        b = std::pow(r, (2.0/3.0));
        // std::cout << "Point p: (" << px << ", " << py << "), Circle center c: (" << cx << ", " << cy << "), Radius r: " << r << std::endl;

        dist2 = (cx - px) * (cx - px) + (cy - py) * (cy - py);
        if (std::abs(dist2) < 1e-6) {dist2 += 1e-6;}
        distc = std::sqrt(std::abs(dist2));
        distnorm = std::exp(c * distc + b);

        loss = a / (1 + std::exp(-c * abs(distc) - b));
        total_loss += loss;

        dl = (a * c * dist2 * distnorm) / (std::pow(std::abs(dist2), (3.0/2.0)) * std::pow(1 + distnorm, 2));
        dx = (cx - px) * dl;
        dy = (cy - py) * dl;
        dr = (2 * a * distnorm) / (3 * std::cbrt(r) * std::pow(1 + distnorm, 2));
        // std::cout << "dx: " << dx << ", dy: " << dy << ", dr: " << dr << std::endl;

        grad(0) += dx;
        grad(1) += dy;
        grad(2) += dr;
    }

    return total_loss / (double)numPoints;
}
