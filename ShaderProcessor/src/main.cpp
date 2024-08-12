#include <KHR/khrplatform.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"
#include "texture.h"
#include "image_processor.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

GLFWwindow* initializeWindow(int width, int height) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

// unsigned int createTexture(int width, int height, GLenum internalFormat) {
//     unsigned int texture;
//     glGenTextures(1, &texture);
//     glBindTexture(GL_TEXTURE_2D, texture);
//     glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RED, GL_FLOAT, NULL);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     return texture;
// }

int main() {
    GLFWwindow* window = initializeWindow(800, 600);
    if (!window) return -1;

    // Shader asciiShader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    Shader asciiShader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    Shader computeShader("../shaders/ascii_compute.glsl");
    
    // Load textures
    unsigned int inputTexture = loadTexture("../assets/frame1358.png");
    unsigned int edgesASCIITexture = loadTexture("../assets/edgesASCII.png");
    unsigned int fillASCIITexture = loadTexture("../assets/fillASCII.png");

    if (edgesASCIITexture == 0 || fillASCIITexture == 0) {
        std::cerr << "Failed to load ASCII textures" << std::endl;
        return -1;
    }
    unsigned int luminanceTexture = createTexture(SCR_WIDTH, SCR_HEIGHT, GL_R16F);
    unsigned int downscaleTexture = createTexture(SCR_WIDTH / 8, SCR_HEIGHT / 8, GL_RGBA16F);
    unsigned int asciiPingTexture = createTexture(SCR_WIDTH, SCR_HEIGHT, GL_RGBA16F);
    unsigned int asciiDogTexture = createTexture(SCR_WIDTH, SCR_HEIGHT, GL_R16F);
    unsigned int normalsTexture = createTexture(SCR_WIDTH, SCR_HEIGHT, GL_RGBA16F);
    unsigned int asciiEdgesTexture = createTexture(SCR_WIDTH, SCR_HEIGHT, GL_R16F);
    unsigned int asciiSobelTexture = createTexture(SCR_WIDTH, SCR_HEIGHT, GL_RG16F);

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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fillASCIITexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, edgesASCIITexture);

    // processImage("../assets/frame1358.png", "output_image.png", asciiShader);
    // processImage("../assets/frame1358.png", "../output/output.png", asciiShader, computeShader);
    processImage("../assets/frame1358.png", "../output/output.png", asciiShader, computeShader, edgesASCIITexture, fillASCIITexture);

    // Clean up
    glDeleteTextures(1, &inputTexture);
    glDeleteTextures(1, &fillASCIITexture);
    glDeleteTextures(1, &edgesASCIITexture);

    glfwTerminate();
    return 0;
}