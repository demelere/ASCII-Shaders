#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include "shader.h"

// void processImage(const char* inputPath, const char* outputPath, Shader& shader);
// void processImage(const char* inputPath, const char* outputPath, Shader& shader, Shader& computeShader);
void processImage(const char* inputPath, const char* outputPath, Shader& shader, Shader& computeShader, unsigned int edgesASCIITexture, unsigned int fillASCIITexture);

unsigned int createTexture(int width, int height, GLenum internalFormat);

#endif