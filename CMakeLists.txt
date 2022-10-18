cmake_minimum_required(VERSION 3.1)

project(engmsc)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory("dep/glad")

add_subdirectory("dep/glfw")

add_subdirectory("dep/glm")

add_subdirectory("dep/iir1")

#Find OpenAL for audio
find_package(OpenAL REQUIRED)

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON  CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_PLY_IMPORTER ON  CACHE BOOL " " FORCE)
set(ASSIMP_NO_EXPORT          ON  CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_TESTS        OFF CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL " " FORCE)
set(ASSIMP_INSTALL            OFF CACHE BOOL " " FORCE)
set(BUILD_SHARED_LIBS         OFF CACHE BOOL " " FORCE)
add_subdirectory("dep/assimp")

add_executable(${PROJECT_NAME}
# Main game engine source files
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
    src/Widget.cpp
    src/Button.cpp
    src/Viewport.cpp
    src/Mesh.cpp
    src/Animation.cpp
    src/AudioProducer.cpp
    src/AudioEvent.cpp
    src/AudioStream.cpp
    src/AudioContext.cpp
    src/AudioFilter.cpp

# Engine Music application source files
    src/engmsc/Gauge.cpp
    src/engmsc/Engine2DRenderer.cpp
    src/engmsc/Engine.cpp
    src/engmsc/EnginePhysics.cpp
    src/engmsc/EngineAudio.cpp
)

# Copy resources to build directory
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    ${CMAKE_CURRENT_BINARY_DIR}/res
    COMMENT "Copying resources into binary directory")

add_dependencies(${PROJECT_NAME} copy_resources)

target_link_libraries(${PROJECT_NAME}
    glfw
    glm
    glad
    assimp
    ${OPENAL_LIBRARY}
    iir::iir_static
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    dep/glfw/include
    dep/glad/include
    dep/glm/
    ${OPENAL_INCLUDE_DIR}
)