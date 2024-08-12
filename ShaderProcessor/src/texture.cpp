#include "texture.h"
#include <glad/glad.h>
// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_wrapper.h"
#include <iostream>

// unsigned int loadTexture(const char* path) {
//     unsigned int texture;
//     glGenTextures(1, &texture);
//     glBindTexture(GL_TEXTURE_2D, texture);

//     int width, height, nrChannels;
//     // unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
//     extern "C" {
//         unsigned char* stbi_load(char const* filename, int* x, int* y, int* comp, int req_comp);
//         void stbi_image_free(void* retval_from_stbi_load);
//     }

//     if (data) {
//         GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
//         glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//         glGenerateMipmap(GL_TEXTURE_2D);
//     } else {
//         std::cerr << "Failed to load texture: " << path << std::endl;
//     }
//     stbi_image_free(data);

//     return texture;
// }

unsigned int loadTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image data
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }

    stbi_image_free(data);

    return texture;
}