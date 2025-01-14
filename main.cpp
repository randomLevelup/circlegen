#include <iostream>
#include <cassert>
#include <limits>

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QImage>

#include "gdcpp.h"
#include "cgio.h"
#include "cgproc.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

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

    // Compute bounds based only on points
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    for (const auto &point : std::get<1>(result)) {
        float px = std::get<0>(point);
        float py = std::get<1>(point);
        minX = std::min(minX, px);
        minY = std::min(minY, py);
        maxX = std::max(maxX, px);
        maxY = std::max(maxY, py);
    }

    int width = static_cast<int>(maxX - minX);
    int height = static_cast<int>(maxY - minY);

    // Scale up the 2D space
    const int scaleFactor = 10;
    width *= scaleFactor;
    height *= scaleFactor;

    // Add padding
    const int padding = scaleFactor * 5;
    width += 2 * padding;
    height += 2 * padding;

    QImage image(width, height, QImage::Format_RGB32);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(image.rect(), Qt::white);

    // Draw circles
    painter.setPen(Qt::black);
    for (const auto &circle : std::get<0>(result)) {
        float cx = (std::get<0>(circle) - minX) * scaleFactor + padding;
        float cy = (std::get<1>(circle) - minY) * scaleFactor + padding;
        float r = std::get<2>(circle) * scaleFactor;
        painter.drawEllipse(QPointF(cx, cy), r, r);
    }

    // Draw points
    painter.setPen(Qt::red);
    const int pointSize = 2; // Scale factor for point size
    for (const auto &point : std::get<1>(result)) {
        float px = (std::get<0>(point) - minX) * scaleFactor + padding;
        float py = (std::get<1>(point) - minY) * scaleFactor + padding;
        painter.drawEllipse(QPointF(px, py), pointSize, pointSize);
    }

    image.save("output.jpg");

    return 0;
}
