cmake_minimum_required(VERSION 3.1)

project(engmsc)

add_subdirectory("dep/glad")

add_subdirectory("dep/glfw")

add_subdirectory("dep/glm")

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Window.cpp
    src/UIRender.cpp
    src/MeshBuilder.cpp
    src/Shaders.cpp
    src/Logger.cpp
    src/Keyboard.cpp
    src/Mouse.cpp
    src/FontRenderer.cpp
    src/stb_image.cpp
)

target_link_libraries(${PROJECT_NAME}
    glfw
    glm
    glad
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    dep/glfw/include
    dep/glad/include
    dep/glm/
)