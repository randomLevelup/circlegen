/**
 * @file cgio.cpp
 * @author Jupiter Westbard
 * @date 03/20/2025
 * @brief input/output implementations for circlegen
 */

#include "circlegen.h"

#include <iostream>
#include <cmath>

#include <cairo.h>

void saveImage(dpixmap pm, dpointlist *points, std::vector<dcircle> &circles) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pm.width, pm.height);
    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    for (int y = 0; y < pm.height; ++y) {
        for (int x = 0; x < pm.width; ++x) {
            dpixel pixel = pm.data[y * pm.width + x];
            cairo_set_source_rgba(cr, pixel.R / 255.0, pixel.G / 255.0, pixel.B / 255.0, 1.0);
            cairo_rectangle(cr, x, y, 1, 1);
            cairo_fill(cr);
        }
    }

    // if (points != nullptr) {
    //     cairo_set_source_rgb(cr, 1, 0, 1);
    //     cairo_set_line_width(cr, 2);
    //     for (const auto& point : *points) {
    //         int x = std::get<0>(point);
    //         int y = std::get<1>(point);
    //         cairo_arc(cr, x, y, 2, 0, 2 * M_PI);
    //         cairo_stroke(cr);
    //     }
    // }

    // Draw circles
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_set_line_width(cr, 1);
    for (const auto& circle : circles) {
        double cx = std::get<0>(circle);
        double cy = std::get<1>(circle);
        double r = std::get<2>(circle);
        cairo_arc(cr, cx, cy, r, 0, 2 * M_PI);
        cairo_stroke(cr);
    }

    cairo_surface_write_to_png(surface, "output.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void breakpointSaveImage(dpixmap *pm, dpointlist &points, dcircle &current, dcircle &last) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pm->width, pm->height);
    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    for (int y = 0; y < pm->height; ++y) {
        for (int x = 0; x < pm->width; ++x) {
            dpixel pixel = pm->data[y * pm->width + x];
            cairo_set_source_rgba(cr, pixel.R / 255.0, pixel.G / 255.0, pixel.B / 255.0, 1.0);
            cairo_rectangle(cr, x, y, 1, 1);
            cairo_fill(cr);
        }
    }

    if (points.size() > 0) {
        cairo_set_source_rgb(cr, 1, 0, 1);
        cairo_set_line_width(cr, 2);
        for (const auto& point : points) {
            int x = std::get<0>(point);
            int y = std::get<1>(point);
            cairo_arc(cr, x, y, 2, 0, 2 * M_PI);
            cairo_stroke(cr);
        }
    }

    // draw last circle
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_arc(cr, std::get<0>(last), std::get<1>(last), std::get<2>(last), 0, 2 * M_PI);
    cairo_stroke(cr);

    // draw current circle
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_arc(cr, std::get<0>(current), std::get<1>(current), std::get<2>(current), 0, 2 * M_PI);
    cairo_stroke(cr);

    cairo_surface_write_to_png(surface, "output.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
