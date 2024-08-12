#ifndef STB_IMAGE_WRAPPER_H
#define STB_IMAGE_WRAPPER_H

#include <cstddef>

unsigned char* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
void stbi_image_free(void* retval_from_stbi_load);

int stbi_write_png(char const* filename, int w, int h, int comp, const void* data, int stride_in_bytes);

#endif // STB_IMAGE_WRAPPER_H