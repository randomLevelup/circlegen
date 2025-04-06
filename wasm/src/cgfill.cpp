/**
 * @file cgfill.cpp
 * @author Jupiter Westbard
 * @date 3/21/2025
 * @brief circlegen coloring implementations
 */

#include "wasm_circlegen.h"

#include <iostream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>

struct dpixelidx { // pixel with index
    uint8_t rgb[3];
    int idx;
}; typedef struct dpixelxy dpixelxy;

struct overlapgroup_ {
    std::unordered_map<uint32_t, std::vector<dpixelidx>> table;
    std::vector<uint32_t> keys;
}; typedef struct overlapgroup_ overlapgroup;

dpixmap quantizeColors(const dpixmap &pm, std::vector<dcircle> &circles) {
    overlapgroup ogroup = {std::unordered_map<uint32_t, std::vector<dpixelidx>>(), std::vector<uint32_t>()};

    for (int y = 0; y < pm.height; ++y) {
        for (int x = 0; x < pm.width; ++x) {
            int index = y * pm.width + x;
            uint8_t r = pm.data[index].R;
            uint8_t g = pm.data[index].G;
            uint8_t b = pm.data[index].B;

            // a pixel's hash key is a n bit number with each bit representing containment in a circle.
            uint32_t key = 0;
            for (int i = 0; i < circles.size(); ++i) {
                float px = static_cast<float>(x);
                float py = static_cast<float>(y);
                float cx = std::get<0>(circles[i]);
                float cy = std::get<1>(circles[i]);
                float r = std::get<2>(circles[i]);
                float dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
                if (dist < r) {
                    key |= 1 << i;
                }
            }

            // create the new dpixel and add it to the hash table
            dpixelidx pixel = {{r, g, b}, index};
            if (ogroup.table.find(key) == ogroup.table.end()) { // group not found
                ogroup.keys.push_back(key);
                ogroup.table[key] = std::vector<dpixelidx>();
            }
            ogroup.table[key].push_back(pixel);
        }
    }

    std::cout << "Found " << ogroup.keys.size() << " fill sections." << std::endl;

    for (auto &key : ogroup.keys) {
        std::vector<dpixelidx> &pixels = ogroup.table[key];

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
        std::sort(pixels.begin(), pixels.end(), [dominant](const dpixelidx &a, const dpixelidx &b) {
            return a.rgb[dominant] < b.rgb[dominant];
        });

        // get median pixel (middle pixel in sorted list)
        dpixelidx median = pixels[pixels.size() / 2];

        // set all pixels to median pixel
        for (auto &pixel : pixels) {
            pixel.rgb[0] = median.rgb[0];
            pixel.rgb[1] = median.rgb[1];
            pixel.rgb[2] = median.rgb[2];
        }
    }

    // create the new dpixmap
    dpixmap res;
    res.width = pm.width;
    res.height = pm.height;
    res.data = new dpixel[pm.width * pm.height];

    for (auto &key : ogroup.keys) {
        for (auto &pixel : ogroup.table[key]) {
            res.data[pixel.idx].R = pixel.rgb[0];
            res.data[pixel.idx].G = pixel.rgb[1];
            res.data[pixel.idx].B = pixel.rgb[2];
        }
    }
    return res;
}
