#include <iostream>
#include <cassert>
#include <limits>

#include "gdcpp.h"

#include "cgio.h"
#include "cgproc.h"
#include "cgfill.h"

int main(int argc, char *argv[]) {
    std::cout << "Starting program..." << std::endl;

    assert(argc >= 2 && argc <= 6);
    const char *filename = argv[1];
    std::cout << "Filename: " << filename << std::endl;

    pathbundle pb = parseSVG(filename);
    std::cout << "Parsed SVG file." << std::endl;

    float res = 3;
    int numcircles = 4;

    for (int i = 2; i < argc; i += 2) {
        if (std::string(argv[i]) == "--res" && i + 1 < argc) {
            res = std::stof(argv[i + 1]);
            std::cout << "Resolution set to: " << res << std::endl;
            res *= 10;
        } else if (std::string(argv[i]) == "--num" && i + 1 < argc) {
            numcircles = std::stoi(argv[i + 1]);
            std::cout << "Number of circles set to: " << numcircles << std::endl;
        }
    }

    dpointlist points = samplePaths(pb, res);
    std::cout << "Sampled paths." << std::endl;

    const float width = std::get<4>(pb);
    const float height = std::get<5>(pb);
    const float scaleFactor = (width + height) / 2.0;
    std::tuple<std::vector<dcircle>, dpointlist> result = generateCircles(points, numcircles, scaleFactor);
    std::cout << "# Points remaining: " << std::get<1>(result).size() << std::endl;

    dpixmap pm = getSVGColorMap(filename);
    std::cout << "Got color map." << std::endl;

    std::vector<dcircle> res_circles = std::get<0>(result);
    dpixmap qpm = quantizeColors(pm, res_circles, scaleFactor);
    std::cout << "Quantized color map." << std::endl;

    const int scale = qpm.scalefactor;
    renderImage(result, qpm, width, height, scale);

    free(pm.data);
    free(qpm.data);
    std::cout << "Program finished." << std::endl;
    return 0;
}
