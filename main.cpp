#include <iostream>
#include <cassert>
#include <matplot/matplot.h>

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

    //

    // render
    auto fig = matplot::figure(true);
    auto ax = fig->current_axes();
    ax->y_axis().reverse(true);
    renderPoints(ax, points);

    fig->save("output.jpg");

    std::cout << "done\n";
    return 0;
}
