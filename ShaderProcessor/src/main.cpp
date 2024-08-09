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

    Shader asciiShader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    
    // Load textures
    unsigned int inputTexture = loadTexture("../assets/frame1358.png");
    unsigned int fillASCIITexture = loadTexture("../assets/fillASCII.png");
    unsigned int edgesASCIITexture = loadTexture("../assets/edgesASCII.png");

    // Set uniforms
    asciiShader.use();
    asciiShader.setInt("inputTexture", 0);
    asciiShader.setInt("FillASCII", 1);
    asciiShader.setInt("EdgesASCII", 2);
    asciiShader.setFloat("_Zoom", 1.0f);
    asciiShader.setVec2("_Offset", 0.0f, 0.0f);
    asciiShader.setInt("_KernelSize", 2);
    asciiShader.setFloat("_Sigma", 2.0f);
    asciiShader.setFloat("_SigmaScale", 1.6f);
    asciiShader.setFloat("_Tau", 1.0f);
    asciiShader.setFloat("_Threshold", 0.005f);
    asciiShader.setBool("_UseDepth", true);
    asciiShader.setFloat("_DepthThreshold", 0.1f);
    asciiShader.setBool("_UseNormals", true);
    asciiShader.setFloat("_NormalThreshold", 0.1f);
    asciiShader.setFloat("_DepthCutoff", 0.0f);
    asciiShader.setInt("_EdgeThreshold", 8);
    asciiShader.setBool("_Edges", true);
    asciiShader.setBool("_Fill", true);
    asciiShader.setFloat("_Exposure", 1.0f);
    asciiShader.setFloat("_Attenuation", 1.0f);
    asciiShader.setBool("_InvertLuminance", false);
    asciiShader.setVec3("_ASCIIColor", 1.0f, 1.0f, 1.0f);
    asciiShader.setVec3("_BackgroundColor", 0.0f, 0.0f, 0.0f);
    asciiShader.setFloat("_BlendWithBase", 0.0f);
    asciiShader.setFloat("_DepthFalloff", 0.0f);
    asciiShader.setFloat("_DepthOffset", 0.0f);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fillASCIITexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, edgesASCIITexture);

    processImage("../assets/input_image.png", "output_image.png", asciiShader);

    // Clean up
    glDeleteTextures(1, &inputTexture);
    glDeleteTextures(1, &fillASCIITexture);
    glDeleteTextures(1, &edgesASCIITexture);

    glfwTerminate();
    return 0;
}