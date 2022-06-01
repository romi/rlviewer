
#include "viewer.h"
#include <vector>

static uint16_t shape_rgb[3] = { 768, 1024, 3 };

int main() {
    viewer_init();
    int pixel_buffer_size = shape_rgb[0] * shape_rgb[1] * shape_rgb[2];
    std::vector<uint8_t> pixels(pixel_buffer_size, 0);

    viewer_load("./215.obj");
//    viewer_grab(&pixels[0], r, lat, lon);
    return 0;
}