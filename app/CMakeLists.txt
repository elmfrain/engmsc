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

#Add NanoGUI
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
add_subdirectory(dep/nanogui)
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")
add_definitions(${NANOGUI_EXTRA_DEFS})

#Add glad for OpenGL
add_subdirectory(dep/glad)

#Add Source Files
add_executable(${PROJECT_NAME}
    src/main.cpp
)

#Link dependancies
target_link_libraries(${PROJECT_NAME}
    sfml-system
    sfml-audio
    nanogui
    ${NANOGUI_EXTRA_LIBS}
    glad
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "include"
    "${NANOGUI_EXTRA_INCS}"
)