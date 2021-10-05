cmake_minimum_required(VERSION 3.10)

project(engmsc)

#Find OpenAL
find_package(OpenAL REQUIRED)
if(NOT OPENAL_FOUND)
    message("Could not find OpenAL!")
else()
    message("OpenAL Version: ${OPENAL_VERSION_STRING}")
endif()

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
    ${OPENAL_LIBRARY}
)

target_include_directories(engmsc PUBLIC
    "include"
    "${OPENAL_INCLUDE_DIR}"
)

#Add testing application
add_subdirectory(app)