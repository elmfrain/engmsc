cmake_minimum_required(VERSION 3.1)

project(engmsc)

add_subdirectory("dep/glfw")

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    glfw
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    dep/glfw/include
)