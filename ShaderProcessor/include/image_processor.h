#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include "shader.h"

void processImage(const char* inputPath, const char* outputPath, Shader& shader, unsigned int edgesASCIITexture, unsigned int fillASCIITexture, Shader* computeShader = nullptr);

unsigned int createTexture(int width, int height, GLenum internalFormat);

#endif