/**
 * @file cgio.cpp
 * @author Jupiter Westbard
 * @date 03/20/2025
 * @brief input/output implementations for circlegen
 */

#include "cgio.h"
#include <iostream>
#include <png.h>
#include <jpeglib.h>
#include <cairo.h>


dpixmap parseImage(const char *filename) {
    dpixmap image = {0, 0, nullptr};
    // Determine file type (crude check, improve this)
    std::string filename_str(filename);
    std::string extension = filename_str.substr(filename_str.find_last_of(".") + 1);

    if (extension == "png") {
        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            std::cerr << "Error: Could not open PNG file " << filename << std::endl;
            return image;
        }

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png) {
            fclose(fp);
            std::cerr << "Error: Could not create PNG read struct" << std::endl;
            return image;
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_read_struct(&png, (png_infopp)NULL, (png_infopp)NULL);
            fclose(fp);
            std::cerr << "Error: Could not create PNG info struct" << std::endl;
            return image;
        }

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &info, (png_infopp)NULL);
            fclose(fp);
            std::cerr << "Error: Error during PNG decoding" << std::endl;
            return image;
        }

        png_init_io(png, fp);
        png_read_info(png, info);

        image.width = png_get_image_width(png, info);
        image.height = png_get_image_height(png, info);
        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);

        if (bit_depth == 16)
            png_set_strip_16(png);

        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);

        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand_gray_1_2_4_to_8(png);

        if (png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);

        if (color_type == PNG_COLOR_TYPE_RGB ||
            color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

        if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);

        png_read_update_info(png, info);

        png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * image.height);
        for (int y = 0; y < image.height; y++) {
            row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
        }

        png_read_image(png, row_pointers);

        image.data = new dpixel[image.width * image.height];
        for (int y = 0; y < image.height; y++) {
            png_byte* row = row_pointers[y];
            for (int x = 0; x < image.width; x++) {
                png_byte* ptr = &(row[x*4]);
                image.data[y * image.width + x].R = ptr[0];
                image.data[y * image.width + x].G = ptr[1];
                image.data[y * image.width + x].B = ptr[2];
            }
             free(row_pointers[y]);
        }
        free(row_pointers);

        png_destroy_read_struct(&png, &info, (png_infopp)NULL);
        fclose(fp);


    } else if (extension == "jpg" || extension == "jpeg") {
        FILE *infile;
        struct jpeg_decompress_struct jcinfo;
        struct jpeg_error_mgr jerr;

        infile = fopen(filename, "rb");
        if (!infile) {
            std::cerr << "Error: Could not open JPG file " << filename << std::endl;
            return image;
        }

        jcinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&jcinfo);
        jpeg_stdio_src(&jcinfo, infile);
        jpeg_read_header(&jcinfo, TRUE);

        jpeg_start_decompress(&jcinfo);

        image.width = jcinfo.output_width;
        image.height = jcinfo.output_height;
        int num_channels = jcinfo.output_components;

        image.data = new dpixel[image.width * image.height];
        JSAMPARRAY buffer = (*jcinfo.mem->alloc_sarray)
            ((j_common_ptr) &jcinfo, JPOOL_IMAGE, image.width * num_channels, 1);

        while (jcinfo.output_scanline < jcinfo.output_height) {
            jpeg_read_scanlines(&jcinfo, buffer, 1);
            for (int x = 0; x < image.width; x++) {
                image.data[(jcinfo.output_scanline - 1) * image.width + x].R = buffer[0][x * num_channels + 0];
                image.data[(jcinfo.output_scanline - 1) * image.width + x].G = buffer[0][x * num_channels + 1];
                image.data[(jcinfo.output_scanline - 1) * image.width + x].B = buffer[0][x * num_channels + 2];
            }
        }

        jpeg_finish_decompress(&jcinfo);
        jpeg_destroy_decompress(&jcinfo);
        fclose(infile);
    } else {
        std::cerr << "Error: Unsupported image format" << std::endl;
    }

    return image;
}

void saveImage(dpixmap pm, dpointlist *points=nullptr) {
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

    if (points != nullptr) {
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_set_line_width(cr, 2);
        for (const auto& point : *points) {
            int x = std::get<0>(point);
            int y = std::get<1>(point);
            cairo_rectangle(cr, x - 1, y - 1, 3, 3);
            cairo_stroke(cr);
        }
    }

    cairo_surface_write_to_png(surface, "output.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

