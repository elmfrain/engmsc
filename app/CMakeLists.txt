cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(engmsc-app VERSION 1.0)

#Copy resource files to current binary folder
if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)

    file(COPY rsc DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    
endif()

#Add NanoGUI
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
add_subdirectory("dep/nanogui")
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")
add_definitions(${NANOGUI_EXTRA_DEFS})

#Add assimp
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_PLY_IMPORTER ON  CACHE BOOL " " FORCE) #Only enable ply importer
set(ASSIMP_NO_EXPORT          ON  CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_TESTS        OFF CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL " " FORCE)
set(ASSIMP_INSTALL            OFF CACHE BOOL " " FORCE)
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
    src/ExhaustConfigCanvas.cpp
)

#Link dependancies
target_link_libraries(${PROJECT_NAME}
    nanogui
    ${NANOGUI_EXTRA_LIBS}
    glad
    assimp
    glm
    engmsc
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "include"
    "${NANOGUI_EXTRA_INCS}"
)