# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(DataLogger)

find_package(Python3 REQUIRED)

set(SPEAKER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main/speaker)
set(AUDIO_WAV ${SPEAKER_DIR}/untitled.wav)
set(AUDIO_HEADER ${SPEAKER_DIR}/audio_data.h)

add_custom_command(
    OUTPUT ${AUDIO_HEADER}
    COMMAND ${CMAKE_COMMAND} -E echo "Generating audio_data.h from untitled.wav"
    COMMAND ${Python3_EXECUTABLE} ${SPEAKER_DIR}/audio_data_generator.py ${AUDIO_WAV} ${AUDIO_HEADER}
    DEPENDS ${AUDIO_WAV} ${SPEAKER_DIR}/audio_data_generator.py
    COMMENT "Running audio_data_generator.py to generate audio_data.h"
)

add_custom_target(generate_audio_data ALL DEPENDS ${AUDIO_HEADER})

add_dependencies(${PROJECT_NAME}.elf generate_audio_data)