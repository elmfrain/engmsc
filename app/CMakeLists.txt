cmake_minimum_required(VERSION 3.10)

project(engmsc-app VERSION 1.0)

add_executable(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)