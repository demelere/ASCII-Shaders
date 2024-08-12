cmake_minimum_required(VERSION 3.10)
project(AsciiShaderProject)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wno-deprecated-declarations)
# add_definitions(-DSTB_IMAGE_IMPLEMENTATION)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(AsciiShader 
    src/main.cpp
    src/glad.c
    src/shader.cpp
    src/texture.cpp
    src/image_processor.cpp
    src/stb_image_wrapper.cpp
)

add_custom_command(TARGET AsciiShader POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:AsciiShader>/assets)

target_link_libraries(AsciiShader 
    OpenGL::GL
    glfw
)