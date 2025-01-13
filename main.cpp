#include <iostream>
#include <cassert>
#include <matplot/matplot.h>
#include "gdcpp.h"

#include "cgio.h"
#include "cgproc.h"

int main(int argc, char const *argv[])
{
    assert(argc == 2 || argc == 4);
    const char *filename = argv[1];

    pathbundle pb = parseSVG(filename);

    float res = 3.0;
    if (argc == 4) {
        if (std::string(argv[2]) == "--res") {
            res = std::stof(argv[3]) / 10;
        }
    }
    else {
        std::cout << "using default res (30)\n";
    }

    dpointlist points = samplePaths(pb, res);

    // do processing lol

    int numcircles = 4;
    std::tuple<std::vector<dcircle>, dpointlist> result = generateCircles(points, numcircles);

    // render
    auto fig = matplot::figure(true);
    auto ax = fig->current_axes();
    ax->y_axis().reverse(true);
    renderCircles(ax, std::get<0>(result));
    renderPoints(ax, std::get<1>(result));

    fig->save("output.jpg");

    std::cout << "done\n";
    return 0;
}
