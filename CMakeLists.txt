cmake_minimum_required(VERSION 3.10)

project(engmsc)

add_subdirectory(dep/iir1)

add_library(engmsc STATIC
    src/IAudioContext.cpp
    src/IAudioProducer.cpp
    src/AudioStream.cpp
    src/KickProducer.cpp
    src/WindProducer.cpp
    src/SoundEvent.cpp

    src/al/ALAudioContext.cpp
)

target_link_libraries(engmsc
    iir::iir_static
)

target_include_directories(engmsc PUBLIC
    "include"
)

#Add testing application
add_subdirectory(app)