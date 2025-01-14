#include <iostream>
#include <cassert>
#include <limits>

#include "gdcpp.h"
#include "cgio.h"
#include "cgproc.h"

int main(int argc, char *argv[]) {

    assert(argc >= 2 && argc <= 6);
    const char *filename = argv[1];

    pathbundle pb = parseSVG(filename);

    float res = 3;
    int numcircles = 4;

    for (int i = 2; i < argc; i += 2) {
        if (std::string(argv[i]) == "--res" && i + 1 < argc) {
            res = std::stof(argv[i + 1]) / 10;
        } else if (std::string(argv[i]) == "--num" && i + 1 < argc) {
            numcircles = std::stoi(argv[i + 1]);
        }
    }

    dpointlist points = samplePaths(pb, res);

    std::tuple<std::vector<dcircle>, dpointlist> result = generateCircles(points, numcircles);

    const int scaleFactor = 10;
    renderImage(result, scaleFactor);

    return 0;
}
