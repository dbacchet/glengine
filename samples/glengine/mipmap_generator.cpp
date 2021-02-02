#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "stb/stb_image_resize.h"

#include "cmdline.h"

#include <string>
#include <sstream>
#include <cstdint>

constexpr int MAX_LEVELS = 15;

int main(int argc, char *argv[]) {

    cmdline::parser cl;
    cl.add<std::string>("input", 'i', "input file name", true, "");
    cl.add<std::string>("output", 'o', "output file name", true, "");
    cl.parse_check(argc, argv);

    std::string input_filename = cl.get<std::string>("input");
    std::string output_prefix = cl.get<std::string>("output");

    int img_width, img_height, num_channels;
    const int out_channels = 4; // force rgba
    // stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels = stbi_load(input_filename.c_str(), &img_width, &img_height, &num_channels, out_channels);
    if (pixels) {
        // for (int i=0; i<10; i++) {
        //     uint8_t *p = pixels + 4*(400*i);
        //     printf("%x%x%x%x ",p[0], p[1], p[2], p[3]);
        // }
        // printf("\n");
        printf("loaded image '%s' - %dx%d %d channels\n", input_filename.c_str(), img_width, img_height, num_channels);
        // generate mipmaps
        uint8_t *out_pixels = (uint8_t *)malloc(img_height * img_width * num_channels / 4);
        int level = 1;
        for (level = 1; level < MAX_LEVELS; level++) {
            int w = img_width / (1 << level);
            int h = img_height / (1 << level);
            if (w < 1 || h < 1) {
                break;
            }
            printf("generate mipmap level %d with resolution %dx%d\n", level, w, h);
            // resize image (starting from the original image)
            // if (!stbir_resize_uint8(pixels, img_width, img_height, 0, out_pixels, w, h, 0, out_channels)) {
            if (!stbir_resize_uint8_srgb(pixels, img_width, img_height, 0, out_pixels, w, h, 0, out_channels,
                                         out_channels == 3 ? STBIR_ALPHA_CHANNEL_NONE : 3, 0)) {
                printf("Error resizing image to %dx%d\n", w, h);
                break;
            }
            // write mipmap level
            std::stringstream out_fname;
            out_fname << output_prefix << "_" << level << ".png";
            printf("    writing image '%s'\n", out_fname.str().c_str());
            stbi_write_png(out_fname.str().c_str(), w, h, out_channels, out_pixels, 0);
        }
        printf("generated %d mipmap levels\n", level);
        stbi_image_free(pixels);
        free(out_pixels);
    }
    return 0;
}

