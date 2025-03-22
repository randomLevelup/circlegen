#include <iostream>
#include <fstream>

#include "argparse.hpp"
#include "gdcpp.h"

#include "circlegen.h"

struct CGArgs : public argparse::Args {
    std::string &img_path  = arg("src_path", "a positional string argument");
};

int main(int argc, char *argv[]) {
    auto args = argparse::parse<CGArgs>(argc, argv);

    std::cout << "Starting program..." << std::endl;
    std::cout << "File path: " << args.img_path << std::endl;

    dpixmap pm;
    try {
        pm = parseImage(args.img_path.c_str());
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    jitteredResample(&pm, 1000, 0.75);

    std::cout << "Parsed image file: "
              << "width: " << pm.width << ", height: " << pm.height << std::endl;

    std::cout << "Applying Sobel filter..." << std::endl;
    dpixmap filtered = sobelFilter(pm);

    std::cout << "Sampling points..." << std::endl;
    dpointlist points = samplePoints(filtered, 300, 0.75);

    std::cout << "\nGenerating circles..." << std::endl;
    std::vector<dcircle> circles = generateCircles(points, &pm, 7);

    std::cout << "\nGenerating fill colors..." << std::endl;
    dpixmap qpm = quantizeColors(pm, circles);

    std::cout << "\nSaving image..." << std::endl;
    saveImage(qpm, &points, circles);
    std::cout << "Saved to 'output.png'." << std::endl;

    delete[] pm.data;
    delete[] filtered.data;
    delete[] qpm.data;
    std::cout << "\nProgram finished." << std::endl;
    return 0;
}
