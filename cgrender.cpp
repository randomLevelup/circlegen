/**
 * @file cgrender.cpp
 * @author Jupiter Westbard
 * @date 1/19/2025
 * @brief circlegen rendering implementation
 */

#include "cgrender.h"
#include <cairo.h>

#define P_PI 3.14159265358979323846

void renderImage(const dbundle &bundle, dpixmap colors, const float w, const float h, const int sf) {
    // Scale up the 2D space
    int width = static_cast<int>(w * sf);
    int height = static_cast<int>(h * sf);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);

    // Fill background with white
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Draw color map
    printf("qpm width: %d, height: %d, stride: %d\n", colors.width, colors.height, colors.stride);
    for (int y = 0; y < colors.height; ++y) {
        for (int x = 0; x < colors.width; ++x) {
            int index = y * colors.stride + x * 4;
            uint8_t b = colors.data[index];     // Blue is at index
            uint8_t g = colors.data[index + 1]; // Green is at index + 1
            uint8_t r = colors.data[index + 2]; // Red is at index + 2
            uint8_t a = colors.data[index + 3]; // Alpha is at index + 3

            cairo_set_source_rgba(cr, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
            cairo_rectangle(cr, (x+2) * sf, (y-2) * sf, sf, sf);
            // cairo_rectangle(cr, x * sf, y * sf, sf, sf);
            // cairo_rectangle(cr, x-1, y-1, 2, 2);
            cairo_fill(cr);
        }
    }

    // Draw circles
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2.5);
    for (const auto &circle : std::get<0>(bundle)) {
        float cx = std::get<0>(circle) * w * sf;
        float cy = std::get<1>(circle) * h * sf;
        float r = std::get<2>(circle) * sf;
        cairo_arc(cr, cx, cy, r, 0, 2 * P_PI);
        cairo_stroke(cr);
    }

    // Draw points
    cairo_set_source_rgb(cr, 1, 0, 0);
    const int pointSize = 2; // Scale factor for point size
    for (const auto &point : std::get<1>(bundle)) {
        float px = std::get<0>(point) * w * sf;
        float py = std::get<1>(point) * h * sf;
        cairo_arc(cr, px, py, pointSize, 0, 2 * P_PI);
        cairo_fill(cr);
    }

    cairo_surface_write_to_png(surface, "output.png");

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
