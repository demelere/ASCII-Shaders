cmake_minimum_required(VERSION 3.10)
project(AsciiShaderProject)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(AsciiShader 
    src/main.cpp
    src/glad.c
    src/shader.cpp
    src/texture.cpp
    src/image_processor.cpp
)

target_link_libraries(AsciiShader 
    OpenGL::GL
    glfw
)