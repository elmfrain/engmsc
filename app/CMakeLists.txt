cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(engmsc-app VERSION 1.0)

#Copy resource files to current binary folder
if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)

    file(COPY rsc DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    
endif()

#Find SMFL
find_package(SFML 2.5 COMPONENTS system audio REQUIRED)

#Find OpenAL
find_package(OpenAL REQUIRED)

#Add NanoGUI
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
add_subdirectory("dep/nanogui")
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")
add_definitions(${NANOGUI_EXTRA_DEFS})

#Add Subdirectory
set(BUILD_SHARED_LIBS ON CACHE BOOL " " FORCE) #Assimp library is huge and it takes a while to link it statically
set(ASSIMP_NO_EXPORT ON CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL " " FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL " " FORCE)
add_subdirectory("dep/assimp")

#Add glad for OpenGL
add_subdirectory("dep/glad")

#Add glm
add_subdirectory("dep/glm")

#Add Source Files
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/MainScreen.cpp
    src/FlywheelRenderer.cpp
)

#Link dependancies
target_link_libraries(${PROJECT_NAME}
    sfml-system
    sfml-audio
    nanogui
    ${NANOGUI_EXTRA_LIBS}
    glad
    assimp
    glm
    engmsc
    openal
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "include"
    "${NANOGUI_EXTRA_INCS}"
    "${OPENAL_INCLUDE_DIR}"
)