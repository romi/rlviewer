#include <vector>
#include <string>
#include "viewer.h"
#include <cstdint>


static uint16_t shape_rgb[3] = { 768, 1024, 3 };

int main(int argc, char** argv) {
    std::string filename = "208.obj";
    if (argc == 2)
        filename = argv[1];

    viewer_init();
    int pixel_buffer_size = shape_rgb[0] * shape_rgb[1] * shape_rgb[2];
    std::vector<uint8_t> pixels(pixel_buffer_size, 0);

    viewer_load(filename.c_str());
    viewer_grab(&pixels[0], 20.0, 0, 0);

    return 0;
}