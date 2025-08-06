idf_component_register(SRCS "speaker_driver.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver)


find_package(Python3 REQUIRED)

set(AUDIO_WAV ${CMAKE_CURRENT_SOURCE_DIR}/untitled.wav)
set(AUDIO_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/audio_data.h)

add_custom_command(
    OUTPUT ${AUDIO_HEADER}
    COMMAND ${CMAKE_COMMAND} -E echo "Generating audio_data.h from untitled.wav"
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/audio_data_generator.py ${AUDIO_WAV} ${AUDIO_HEADER}
    DEPENDS ${AUDIO_WAV} ${CMAKE_CURRENT_SOURCE_DIR}/audio_data_generator.py
    COMMENT "Running audio_data_generator.py to generate audio_data.h"
)

set_source_files_properties(${AUDIO_HEADER} PROPERTIES GENERATED TRUE)
target_sources(${COMPONENT_LIB} PRIVATE ${AUDIO_HEADER})