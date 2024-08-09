#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"
#include "texture.h"
#include "image_processor.h"

GLFWwindow* initializeWindow(int width, int height);

int main() {
    GLFWwindow* window = initializeWindow(800, 600);
    if (!window) return -1;

    Shader asciiShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    
    // Load textures
    unsigned int inputTexture = loadTexture("input_image.png");
    unsigned int fillASCIITexture = loadTexture("fillASCII.png");

    // Set uniforms
    asciiShader.use();
    asciiShader.setInt("inputTexture", 0);
    asciiShader.setInt("FillASCII", 1);
    asciiShader.setFloat("_Exposure", 1.0f);
    asciiShader.setFloat("_Attenuation", 1.0f);
    asciiShader.setBool("_InvertLuminance", false);
    asciiShader.setVec3("_ASCIIColor", 1.0f, 1.0f, 1.0f);
    asciiShader.setVec3("_BackgroundColor", 0.0f, 0.0f, 0.0f);
    asciiShader.setFloat("_BlendWithBase", 0.0f);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fillASCIITexture);

    processImage("input_image.png", "output_image.png", asciiShader);

    // Clean up
    glDeleteTextures(1, &inputTexture);
    glDeleteTextures(1, &fillASCIITexture);

    glfwTerminate();
    return 0;
}

GLFWwindow* initializeWindow(int width, int height) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(width, height, "ASCII Shader", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    return window;
}