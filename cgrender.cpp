/**
 * @file cgrender.cpp
 * @author Jupiter Westbard
 * @date 1/19/2025
 * @brief circlegen rendering implementation
 */

#include <iostream>

#include "cgrender.h"
#include <cairo.h>

#define P_PI 3.14159265358979323846

void renderImage(const dbundle &bundle, pathbundle &pb, dpixmap &colors,
                 bool rpoints, bool rcircles, bool rfill) {
    std::cout << "Rendering image..." << std::endl;

    int pix_width = colors.width;
    int pix_height = colors.height;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pix_width, pix_height);
    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    int sf = colors.scalefactor;
    std::cout << "QPM width: " << pix_width << ", height: " << pix_height << ", stride: " << colors.stride << std::endl;

    if (rfill) {
        for (int y = 0; y < pix_height; ++y) {
            for (int x = 0; x < pix_width; ++x) {
                int index = y * colors.stride + x * 4;
                uint8_t b = colors.data[index];
                uint8_t g = colors.data[index + 1];
                uint8_t r = colors.data[index + 2];
                uint8_t a = colors.data[index + 3];
                
                cairo_set_source_rgba(cr, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
                cairo_rectangle(cr, x, y, 1, 1);
                cairo_fill(cr);
            }
        }
    }

    float radratio = static_cast<float>(2 * pix_width) / static_cast<float>(pix_width + pix_height);

    if (rcircles) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 2.5);
        for (const auto &circle : std::get<0>(bundle)) {
            float cx = std::get<0>(circle) * pix_width;
            float cy = std::get<1>(circle) * pix_height;
            float r = std::get<2>(circle) * colors.scalefactor * radratio;
            cairo_save(cr);
            cairo_translate(cr, cx, cy);
            cairo_scale(cr, 1.0, ((float)colors.height / (float)colors.width));
            cairo_arc(cr, 0, 0, r, 0, 2 * P_PI);
            cairo_restore(cr);
            cairo_stroke(cr);
        }
    }

    if (rpoints) {
        cairo_set_source_rgb(cr, 1, 0, 0);
        const int pointSize = 5;
        for (const auto &point : std::get<1>(bundle)) {
            float px = std::get<0>(point) * pix_width;
            float py = std::get<1>(point) * pix_height;
            cairo_arc(cr, px, py, pointSize, 0, 2 * P_PI);
            cairo_fill(cr);
        }
    }

    cairo_surface_write_to_png(surface, "output.png");

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    std::cout << "...Rendering done" << std::endl;
}
