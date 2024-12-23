#include <iostream>
#include <cassert>
#include <matplot/matplot.h>

#include "cgio.h"
#include "cgproc.h"

int main(int argc, char const *argv[])
{
    assert(argc == 2);
    const char *filename = argv[1];

    pathbundle pb = parseSVG(filename);

    dpointlist points = samplePaths(pb, 10);

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
