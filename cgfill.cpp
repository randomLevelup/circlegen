/**
 * @file cgfill.cpp
 * @author Jupiter Westbard
 * @date 1/15/2025
 * @brief circlegen coloring implementations
 */

#include "cgfill.h"

#include <iostream>
#include <cstdint>
#include <cstring>

dpixmap getSVGColorMap(const char *filename);

dpixmap getSVGColorMap(const char *filename) {
    dpixmap res = {nullptr, 0, 0, 0};

    GError *error = NULL;
    RsvgHandle *handle = rsvg_handle_new_from_file(filename, &error);
    if (!handle) {
        std::cerr << "Error: Unable to create RsvgHandle from file " << filename << ": " << error->message << std::endl;
        g_error_free(error);
        return res;
    }

    RsvgDimensionData dimensions;
    rsvg_handle_get_dimensions(handle, &dimensions);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dimensions.width, dimensions.height);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        std::cerr << "Error: Unable to create cairo surface" << std::endl;
        g_object_unref(handle);
        return res;
    }

    cairo_t *cr = cairo_create(surface);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        std::cerr << "Error: Unable to create cairo context" << std::endl;
        cairo_surface_destroy(surface);
        g_object_unref(handle);
        return res;
    }

    RsvgRectangle viewport = {0, 0, static_cast<double>(dimensions.width), static_cast<double>(dimensions.height)};
    if (!rsvg_handle_render_document(handle, cr, &viewport, &error)) {
        std::cerr << "Error: Unable to render SVG document: " << error->message << std::endl;
        g_error_free(error);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        g_object_unref(handle);
        return res;
    }

    int stride = cairo_image_surface_get_stride(surface);
    int dataSize = stride * dimensions.height;
    res.data = new uint8_t[dataSize];
    std::memcpy(res.data, cairo_image_surface_get_data(surface), dataSize);
    res.stride = stride;
    res.width = dimensions.width;
    res.height = dimensions.height;

    for (int i = 0; i < dataSize; i += 4) {
        uint8_t r = res.data[i];
        uint8_t g = res.data[i + 1];
        uint8_t b = res.data[i + 2];
        uint8_t a = res.data[i + 3];
        std::cout << "r: " << (int)r << ", g: " << (int)g << ", b: " << (int)b << ", a: " << (int)a << std::endl;
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);

    return res;
}
