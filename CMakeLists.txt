cmake_minimum_required(VERSION 3.10)

project(engmsc)

add_library(engmsc STATIC
    src/IAudioContext.cpp
    src/AudioStream.cpp
    src/BandFilter.cpp

    src/al/ALAudioContext.cpp
)

target_include_directories(engmsc PUBLIC
    "include"
)

#Add testing application
add_subdirectory(app)