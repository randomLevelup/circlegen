/**
 * @file cgfill.cpp
 * @author Jupiter Westbard
 * @date 1/15/2025
 * @brief circlegen coloring implementations
 */

#include "cgfill.h"

#include <iostream>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cassert>

dpixmap quantizeColors(const dpixmap &pm, std::vector<dcircle> &circles, float c_sf);
dpixmap getSVGColorMap(const char *filename);

dpixmap quantizeColors(const dpixmap &pm, std::vector<dcircle> &circles, float c_sf) {
    if (circles.size() >= 32) {
        std::cerr << "Error: The number of circles (" << circles.size() << ") exceeds the maximum allowed (31)." << std::endl;
        assert(circles.size() < 32);
    }
    overlapgroup ogroup = {std::unordered_map<uint32_t, std::vector<dpixel>>(), std::vector<uint32_t>()};
    std::cout << "computing fill with circle scale factor: " << c_sf << std::endl;

    std::cout << "pm width: " << pm.width << ", height: " << pm.height << ", stride: " << pm.stride << std::endl;
    for (int y = 0; y < pm.height; ++y) {
        for (int x = 0; x < pm.width; ++x) {
            int index = y * pm.stride + x * 4; // Assuming 4 bytes per pixel (e.g., RGBA)
            uint8_t r = pm.data[index + 2]; // Red is at index + 2
            uint8_t g = pm.data[index + 1]; // Green is at index + 1
            uint8_t b = pm.data[index];     // Blue is at index
            uint8_t a = pm.data[index + 3]; // Alpha is at index + 3

            // crush alpha (if alpha is < 128, set it to 1 and make the pixel white)
            if (a <= 128) {
                r = 255; g = 255; b = 255;
                a = 255;
            }

            // Process the pixel (r, g, b, a) at position (x, y)
            // A pixel's hash key is a n bit number with each bit representing containment in a circle. 
            uint32_t key = 0;
            for (int i = 0; i < circles.size(); ++i) {
                float px = static_cast<float>(x);
                float py = static_cast<float>(y);
                float cx = std::get<0>(circles[i]) * c_sf;
                float cy = std::get<1>(circles[i]) * c_sf;
                float r = std::get<2>(circles[i]);
                float dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
                if (dist < r) {
                    key |= 1 << i;
                }
            }

            // create the new dpixel and add it to the hash table
            dpixel pixel = {{r, g, b}, a, index};
            if (ogroup.table.find(key) == ogroup.table.end()) {
                ogroup.keys.push_back(key);
            }
            ogroup.table[key].push_back(pixel);
        }
    }

    // print out ogroup sizes
    std::cout << "number of ogroups: " << ogroup.keys.size() << std::endl;

    for (auto &key : ogroup.keys) {
        std::vector<dpixel> &pixels = ogroup.table[key];
        
        // find the dominant channel (the r, g, or b channel with the highest range)
        int rmin = 255, rmax = 0;
        int gmin = 255, gmax = 0;
        int bmin = 255, bmax = 0;
        for (auto &pixel : pixels) {
            if (pixel.rgb[0] < rmin) rmin = pixel.rgb[0];
            if (pixel.rgb[0] > rmax) rmax = pixel.rgb[0];
            if (pixel.rgb[1] < gmin) gmin = pixel.rgb[1];
            if (pixel.rgb[1] > gmax) gmax = pixel.rgb[1];
            if (pixel.rgb[2] < bmin) bmin = pixel.rgb[2];
            if (pixel.rgb[2] > bmax) bmax = pixel.rgb[2];
        }
        int rrange = rmax - rmin;
        int grange = gmax - gmin;
        int brange = bmax - bmin;
        int dominant = 0;
        if (grange > rrange) dominant = 1;
        if (brange > rrange && brange > grange) dominant = 2;

        // sort pixels by dominant channel
        std::sort(pixels.begin(), pixels.end(), [dominant](const dpixel &a, const dpixel &b) {
            return a.rgb[dominant] < b.rgb[dominant];
        });

        // get median pixel (middle pixel in sorted list)
        dpixel median = pixels[pixels.size() / 2];
        // printf("group %d: %d pixels, color: %d %d %d\n", key, (int)pixels.size(), median.rgb[0], median.rgb[1], median.rgb[2]);

        // set all pixels to median pixel
        for (auto &pixel : pixels) {
            pixel.rgb[0] = median.rgb[0];
            pixel.rgb[1] = median.rgb[1];
            pixel.rgb[2] = median.rgb[2];
            // continue;
        }
    }

    // create new dpixmap with quantized colors
    dpixmap res = {new uint8_t[pm.stride * pm.height], pm.width, pm.height, pm.stride};
    for (auto &key : ogroup.keys) {
        for (auto &pixel : ogroup.table[key]) {
            res.data[pixel.idx] = pixel.rgb[2];     // Blue is at index
            res.data[pixel.idx + 1] = pixel.rgb[1]; // Green is at index + 1
            res.data[pixel.idx + 2] = pixel.rgb[0]; // Red is at index + 2
            res.data[pixel.idx + 3] = 255;          // Alpha is at index + 3
        }
    }
    res.width = pm.width;
    res.height = pm.height;
    res.stride = pm.stride;
    res.scalefactor = pm.scalefactor;
    return res;
}

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

    // scale factor is the multiplicand to make the larger dimension at least 2000
    int scaleFactor = (int)ceil(2000.0 / std::max(dimensions.width, dimensions.height));
    std::cout << "Colormap ScaleFactor: " << scaleFactor << std::endl;

    int scaledWidth = dimensions.width * scaleFactor;
    int scaledHeight = dimensions.height * scaleFactor;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, scaledWidth, scaledHeight);
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

    // cairo_scale(cr, scaleFactor, scaleFactor);

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
    int dataSize = stride * scaledHeight;
    uint8_t *surface_data = cairo_image_surface_get_data(surface);
    res.data = new uint8_t[dataSize];
    std::memcpy(res.data, surface_data, dataSize);
    res.stride = stride;
    res.width = scaledWidth;
    res.height = scaledHeight;
    res.scalefactor = scaleFactor;

    cairo_surface_mark_dirty(surface);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);

    return res;
}
