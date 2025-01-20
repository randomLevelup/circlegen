#include <iostream>
#include <cassert>
#include <limits>

#include "gdcpp.h"

#include "cgio.h"
#include "cgproc.h"
#include "cgfill.h"
#include "cgrender.h"

#define RES_SCALE 0.07142
#define RES_OFFSET 0.42857

int main(int argc, char *argv[]) {
    std::cout << "Starting program..." << std::endl;

    assert(argc >= 2 && argc <= 6);
    const char *filename = argv[1];
    std::cout << "Filename: " << filename << std::endl;

    pathbundle pb = parseSVG(filename);
    std::cout << "Parsed SVG file." << std::endl;
    printf("viewbox: [%f x %f] - (%f, %f)\n", std::get<4>(pb), std::get<5>(pb), std::get<6>(pb), std::get<7>(pb));

    float res = 20;
    int numcircles = 8;

    for (int i = 2; i < argc; i += 2) {
        if (std::string(argv[i]) == "--res" && i + 1 < argc) {
            res = std::stof(argv[i + 1]);
            std::cout << "Resolution set to: " << res << std::endl;
            res *= 10;
        } else if (std::string(argv[i]) == "--num" && i + 1 < argc) {
            numcircles = std::stoi(argv[i + 1]);
            res *= (RES_SCALE * static_cast<float>(numcircles)) + RES_OFFSET;
            std::cout << "Number of circles set to: " << numcircles << std::endl;
        }
    }

    dpointlist points = samplePaths(pb, res);
    std::cout << "Sampled paths." << std::endl;

    const float width = std::get<4>(pb);
    const float height = std::get<5>(pb);
    const float circles_scalefactor = (width + height) / 2.0;
    std::tuple<std::vector<dcircle>, dpointlist> result = generateCircles(points, numcircles, circles_scalefactor);
    std::cout << "# Points remaining: " << std::get<1>(result).size() << std::endl;

    dpixmap pm = getSVGColorMap(filename);
    std::cout << "Got color map." << std::endl;

    std::vector<dcircle> res_circles = std::get<0>(result);
    dpixmap qpm = quantizeColors(pm, res_circles, circles_scalefactor);
    std::cout << "Quantized color map." << std::endl;

    renderImage(result, pb, qpm);

    // free pm and qpm if not null
    if (pm.data != nullptr) { delete[]pm.data; pm.data = nullptr; }
    if (qpm.data != nullptr) { delete[]qpm.data; qpm.data = nullptr;}

    std::cout << "Program finished." << std::endl;
    return 0;
}
