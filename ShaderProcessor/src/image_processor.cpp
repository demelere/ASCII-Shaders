#include "image_processor.h"
#include <KHR/khrplatform.h>
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include "stb_image_write.h"
#include "stb_image.h"
#include <GL/glext.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

unsigned int quadVAO, quadVBO;
void setupQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

int getPassIndex(const char* passName) {
    // Map pass names to indices
    if (strcmp(passName, "PS_Luminance") == 0) return 0;
    if (strcmp(passName, "PS_Downscale") == 0) return 1;
    if (strcmp(passName, "PS_HorizontalBlur") == 0) return 2;
    if (strcmp(passName, "PS_VerticalBlurAndDifference") == 0) return 3;
    if (strcmp(passName, "PS_CalculateNormals") == 0) return 4;
    if (strcmp(passName, "PS_EdgeDetect") == 0) return 5;
    if (strcmp(passName, "PS_HorizontalSobel") == 0) return 6;
    if (strcmp(passName, "PS_VerticalSobel") == 0) return 7;
    if (strcmp(passName, "PS_EndPass") == 0) return 8;
    return -1; // Invalid pass name
}

void renderPass(Shader& shader, const char* passName) {
    shader.use();
    shader.setInt("pass", getPassIndex(passName));
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

unsigned int createTexture(int width, int height, GLenum internalFormat) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texture;
}

void checkOutputDirectory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        std::cerr << "Cannot access " << path << ": " << strerror(errno) << std::endl;
    } else if (!(info.st_mode & S_IFDIR)) {
        std::cerr << path << " is not a directory" << std::endl;
    } else {
        std::cout << "Output directory is accessible: " << path << std::endl;
    }
}

void checkOpenGLError(const std::string& location) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error at " << location << ": " << err << std::endl;
    }
}

void createOutputDirectory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        // Directory does not exist, attempt to create it
        if (mkdir(path.c_str(), 0777) == -1) {
            std::cerr << "Error creating directory " << path << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Output directory created: " << path << std::endl;
        }
    } else if (!(info.st_mode & S_IFDIR)) {
        std::cerr << path << " is not a directory" << std::endl;
    } else {
        std::cout << "Output directory is accessible: " << path << std::endl;
    }
}

// void processImage(const char* inputPath, const char* outputPath, Shader& shader) {
// void processImage(const char* inputPath, const char* outputPath, Shader& shader, Shader& computeShader) {
// void processImage(const char* inputPath, const char* outputPath, Shader& shader, Shader& computeShader, unsigned int edgesASCIITexture, unsigned int fillASCIITexture) {
void processImage(const char* inputPath, const char* outputPath, Shader& shader, unsigned int edgesASCIITexture, unsigned int fillASCIITexture, Shader* computeShader) {
    // Load input image
    int width, height, channels;
    unsigned char* inputData = stbi_load(inputPath, &width, &height, &channels, 0);
    if (!inputData) {
        std::cerr << "Failed to load input image" << std::endl;
        return;
    }

    // checkOutputDirectory("../output/");
    createOutputDirectory("../output/");

    // Create framebuffer
   // Create framebuffer
unsigned int fbo;
glGenFramebuffers(1, &fbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);

// Create and attach a color attachment texture
unsigned int colorAttachment;
glGenTextures(1, &colorAttachment);
glBindTexture(GL_TEXTURE_2D, colorAttachment);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment, 0);

// Check framebuffer completeness
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Framebuffer is not complete!" << std::endl;
    return;
}
std::cout << "Framebuffer created successfully." << std::endl;

    // Create textures for each pass
unsigned int luminanceTexture = createTexture(width, height, GL_R16F);
if (luminanceTexture == 0) {
    std::cerr << "Failed to create luminance texture" << std::endl;
    return;
}

unsigned int downscaleTexture = createTexture(width / 8, height / 8, GL_RGBA16F);
if (downscaleTexture == 0) {
    std::cerr << "Failed to create downscale texture" << std::endl;
    return;
}

unsigned int asciiPingTexture = createTexture(width, height, GL_RGBA16F);
if (asciiPingTexture == 0) {
    std::cerr << "Failed to create ASCII ping texture" << std::endl;
    return;
}

unsigned int asciiDogTexture = createTexture(width, height, GL_R16F);
if (asciiDogTexture == 0) {
    std::cerr << "Failed to create ASCII DoG texture" << std::endl;
    return;
}

unsigned int normalsTexture = createTexture(width, height, GL_RGBA16F);
if (normalsTexture == 0) {
    std::cerr << "Failed to create normals texture" << std::endl;
    return;
}

unsigned int asciiEdgesTexture = createTexture(width, height, GL_R16F);
if (asciiEdgesTexture == 0) {
    std::cerr << "Failed to create ASCII edges texture" << std::endl;
    return;
}

unsigned int asciiSobelTexture = createTexture(width, height, GL_RG16F);
if (asciiSobelTexture == 0) {
    std::cerr << "Failed to create ASCII Sobel texture" << std::endl;
    return;
}

// Create texture to render to
unsigned int outputTexture;
glGenTextures(1, &outputTexture);
glBindTexture(GL_TEXTURE_2D, outputTexture);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to create output texture" << std::endl;
    checkOpenGLError("glTexImage2D for outputTexture");
    return;
}

    // Use the passed edgesASCIITexture and fillASCIITexture
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, edgesASCIITexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, fillASCIITexture);

    // Set the uniforms for these textures in the shader
    shader.use();
    shader.setInt("EdgesASCII", 2);
    shader.setInt("FillASCII", 3);

    // Create texture to render to
    // unsigned int outputTexture;
    // glGenTextures(1, &outputTexture);
    // glBindTexture(GL_TEXTURE_2D, outputTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        return;
    }
     // Render passes
    // Pass 1: Luminance
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, luminanceTexture, 0);
    renderPass(shader, "PS_Luminance");

    // Pass 2: Downscale
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, downscaleTexture, 0);
    renderPass(shader, "PS_Downscale");

    // Pass 3: Horizontal Blur
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, asciiPingTexture, 0);
    renderPass(shader, "PS_HorizontalBlur");

    // Pass 4: Vertical Blur and Difference
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, asciiDogTexture, 0);
    renderPass(shader, "PS_VerticalBlurAndDifference");

    // Pass 5: Calculate Normals
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalsTexture, 0);
    renderPass(shader, "PS_CalculateNormals");

    // Pass 6: Edge Detect
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, asciiEdgesTexture, 0);
    renderPass(shader, "PS_EdgeDetect");

    // Pass 7: Horizontal Sobel
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, asciiPingTexture, 0);
    renderPass(shader, "PS_HorizontalSobel");

    // Pass 8: Vertical Sobel
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, asciiSobelTexture, 0);
    renderPass(shader, "PS_VerticalSobel");

    // Specify draw buffers
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6};
    glDrawBuffers(7, drawBuffers);

    // // Check framebuffer completeness
    // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    //     std::cout << "Framebuffer is not complete!" << std::endl;
    // }

    // Pass 9: Render ASCII (This will be a compute shader pass, we'll handle it separately)
    if (computeShader) {
    // Compute shader pass
    computeShader->use();

    // Bind input textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, asciiSobelTexture);
    computeShader->setInt("Sobel", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, downscaleTexture);
    computeShader->setInt("Downscale", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, edgesASCIITexture);
    computeShader->setInt("EdgesASCII", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, fillASCIITexture);
    computeShader->setInt("FillASCII", 3);

    // Set uniforms
    computeShader->setInt("_EdgeThreshold", 8);
    computeShader->setBool("_Edges", true);
    computeShader->setBool("_Fill", true);
    computeShader->setFloat("_Exposure", 1.0f);
    computeShader->setFloat("_Attenuation", 1.0f);
    computeShader->setBool("_InvertLuminance", false);
    computeShader->setVec3("_ASCIIColor", 1.0f, 1.0f, 1.0f);
    computeShader->setVec3("_BackgroundColor", 0.0f, 0.0f, 0.0f);
    computeShader->setFloat("_BlendWithBase", 0.0f);

    // Bind image textures
    glBindImageTexture(0, asciiSobelTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Dispatch compute shader
    glDispatchCompute((width + 7) / 8, (height + 7) / 8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
} else {
    // Fallback fragment shader pass
    shader.use();

    // Bind input textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, asciiSobelTexture);
    shader.setInt("Sobel", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, downscaleTexture);
    shader.setInt("Downscale", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, edgesASCIITexture);
    shader.setInt("EdgesASCII", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, fillASCIITexture);
    shader.setInt("FillASCII", 3);

    // Set uniforms
    shader.setInt("_EdgeThreshold", 8);
    shader.setBool("_Edges", true);
    shader.setBool("_Fill", true);
    shader.setFloat("_Exposure", 1.0f);
    shader.setFloat("_Attenuation", 1.0f);
    shader.setBool("_InvertLuminance", false);
    shader.setVec3("_ASCIIColor", 1.0f, 1.0f, 1.0f);
    shader.setVec3("_BackgroundColor", 0.0f, 0.0f, 0.0f);
    shader.setFloat("_BlendWithBase", 0.0f);

    // Render to output texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);
    glViewport(0, 0, width, height);
    
    // Draw fullscreen quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

    // Final Pass
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);
    // renderPass(shader, "PS_EndPass");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    shader.use();
    shader.setInt("pass", getPassIndex("PS_EndPass"));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    // renderPass(shader, "PS_EndPass");   
    std::cout << "Starting final render pass..." << std::endl;
    renderPass(shader, "PS_EndPass");
    std::cout << "Final render pass completed." << std::endl;

    // Read pixels
    std::vector<unsigned char> outputData(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, outputData.data()); 
    if (!inputData) {
        std::cerr << "Failed to load input image: " << inputPath << std::endl;
        return;
    }
    std::cout << "Input image loaded successfully." << std::endl;

    // Save output image
    std::cout << "Attempting to write output image to: " << outputPath << std::endl;
    std::cout << "Image dimensions: " << width << "x" << height << std::endl;
    std::cout << "Output data size: " << outputData.size() << std::endl;
    stbi_flip_vertically_on_write(true);
    stbi_write_png(outputPath, width, height, 3, outputData.data(), width * 3);
    if (!stbi_write_png(outputPath, width, height, 3, outputData.data(), width * 3)) {
        std::cerr << "Failed to write output image: " << outputPath << std::endl;
    } else {
        std::cout << "Output image saved successfully: " << outputPath << std::endl;
    }

    // Set up vertex data for a fullscreen quad
    float vertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render
    glViewport(0, 0, width, height);
    shader.use();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Read pixels
    // std::vector<unsigned char> outputData(width * height * 3);
    // glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, outputData.data());

    // Save output image
    stbi_flip_vertically_on_write(true);
    stbi_write_png(outputPath, width, height, 3, outputData.data(), width * 3);

    // Clean up
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &luminanceTexture);
    glDeleteTextures(1, &downscaleTexture);
    glDeleteTextures(1, &asciiPingTexture);
    glDeleteTextures(1, &asciiDogTexture);
    glDeleteTextures(1, &normalsTexture);
    glDeleteTextures(1, &asciiEdgesTexture);
    glDeleteTextures(1, &asciiSobelTexture);
    glDeleteTextures(1, &outputTexture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    stbi_image_free(inputData);
}