cmake_minimum_required(VERSION 3.10)

project(engmsc-app VERSION 1.0)

#Copy resource files to current binary folder
if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)

    file(COPY rsc DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    
endif()

#Find SMFL
find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)

#Add Source Files
add_executable(${PROJECT_NAME}
    src/main.cpp
)

#Link dependancies
target_link_libraries(${PROJECT_NAME}
    sfml-system
    sfml-audio
)
target_include_directories(${PROJECT_NAME} PRIVATE
    include
)