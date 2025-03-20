#include <iostream>
#include <fstream>
#include <cassert>
#include <limits>

#include "gdcpp.h"

#include "cgio.h"
#include "cgproc.h"
#include "cgfill.h"
#include "cgrender.h"

#define RES_SCALE 0.07142
#define RES_OFFSET 0.42857

static int suppressCout();

int main(int argc, char *argv[]) {
    float res = 20;
    int numcircles = 8;
    int verbosity = 1;
    bool rpoints = false;
    bool rcircles = true;
    bool rfill = true;
    bool nogen = false;

    for (int i = 2; i < argc; i += 2) {
        if (std::string(argv[i]) == "--res" && i + 1 < argc) {
            res = std::stof(argv[i + 1]);
            res *= 10;
        } else if (std::string(argv[i]) == "--num" && i + 1 < argc) {
            numcircles = std::stoi(argv[i + 1]);
            res *= (RES_SCALE * static_cast<float>(numcircles)) + RES_OFFSET;
        } else if (std::string(argv[i]) == "--debug" && i + 1 < argc) {
            verbosity = std::stoi(argv[i + 1]);
            if (verbosity <= 0) {
                std::cout.setstate(std::ios_base::failbit);
            }
        } else if (std::string(argv[i]) == "--rpoints" && i + 1 < argc) {
            rpoints = (std::string(argv[i + 1]) == "true");
        } else if (std::string(argv[i]) == "--rcircles" && i + 1 < argc) {
            rcircles = (std::string(argv[i + 1]) == "true");
        } else if (std::string(argv[i]) == "--rfill" && i + 1 < argc) {
            rfill = (std::string(argv[i + 1]) == "true");
        } else if (std::string(argv[i]) == "--nogen") {
            nogen = true;
            i -= 1; // adjust to not skip next argument
        }
    }
    std::cout << "Starting program..." << std::endl;
    std::cout << "Resolution set to: " << res << std::endl;
    std::cout << "Number of circles set to: " << numcircles << std::endl;
    std::cout << "Verbosity set to: " << verbosity << std::endl;

    const char *filename = argv[1];
    std::cout << "File path: " << filename << std::endl;

    pathbundle pb = parseSVG(filename);
    std::cout << "Parsed SVG file: "
              << "viewbox: [" << std::get<4>(pb) << " x " << std::get<5>(pb) << "] - (" << std::get<6>(pb) << ", " << std::get<7>(pb) << ")" << std::endl;


    dpointlist points = samplePaths(pb, res);
    std::cout << "Sampled paths." << std::endl;

    const float width = std::get<4>(pb);
    const float height = std::get<5>(pb);
    const float minX = std::get<6>(pb);
    const float minY = std::get<7>(pb);
    const float circles_scalefactor = (width + height) / 2.0;
    std::tuple<std::vector<dcircle>, dpointlist> result;

    if (!nogen) {
        result = generateCircles(points, numcircles, circles_scalefactor, verbosity);
    } else {
        std::vector<dcircle> dummy;
        result = std::make_tuple(dummy, points);
    }
    std::cout << "# Points remaining: " << std::get<1>(result).size() << std::endl;

    dpixmap pm = getSVGColorMap(filename, minX, minY);
    std::cout << "Got color map." << std::endl;

    std::vector<dcircle> res_circles = std::get<0>(result);
    dpixmap qpm = quantizeColors(pm, res_circles, circles_scalefactor);
    std::cout << "Quantized color map." << std::endl;

    renderImage(result, pb, pm, rpoints, rcircles, rfill);

    // free pm and qpm if not null
    if (pm.data != nullptr) { delete[]pm.data; pm.data = nullptr; }
    if (qpm.data != nullptr) { delete[]qpm.data; qpm.data = nullptr;}

    std::cout << "Program finished." << std::endl;
    return 0;
}
