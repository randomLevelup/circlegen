#include <iostream>
#include <cassert>
#include <limits>

#include "gdcpp.h"
#include "cgio.h"
#include "cgproc.h"

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

    std::vector<dcircle> dummycirclelist = std::vector<dcircle>();
    std::tuple<std::vector<dcircle>, dpointlist> result = std::make_tuple(dummycirclelist, points);
    // std::tuple<std::vector<dcircle>, dpointlist> result = generateCircles(points, numcircles);

    // PixelStream pstream = getSVGStream(filename);

    std::cout << "Number of points: " << std::get<1>(result).size() << std::endl;

    const float width = std::get<4>(pb);
    const float height = std::get<5>(pb);
    const int scaleFactor = 10;
    std::cout << "Rendering image with width: " << width << ", height: " << height << ", scale factor: " << scaleFactor << std::endl;
    renderImage(result, width, height, scaleFactor);

    std::cout << "Program finished." << std::endl;
    return 0;
}
