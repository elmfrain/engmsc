cmake_minimum_required(VERSION 3.2)

project("glad")

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

add_library(${PROJECT_NAME} STATIC
    src/glad.c
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${CMAKE_DL_LIBS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "include"
)