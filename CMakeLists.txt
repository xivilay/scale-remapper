cmake_minimum_required(VERSION 3.1.5)

set(BINARY_NAME "MidiScaleRemapper")
set(VERSION "0.0.1")
set(JUCE_DIR "../JUCE")
set(JUCE_MODULES_DIR "${JUCE_DIR}/modules")
set(SRC_DIR "./src")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(${BINARY_NAME} VERSION ${VERSION})

add_subdirectory(${JUCE_DIR} "juce")

juce_add_plugin(${BINARY_NAME}
    COMPANY_NAME "JUCE"
    PRODUCT_NAME "${BINARY_NAME}"
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    FORMATS VST3
)

juce_generate_juce_header(${BINARY_NAME})

target_sources(${BINARY_NAME} PRIVATE
    "${SRC_DIR}/Main.cpp"
)

target_compile_definitions(${BINARY_NAME} PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(${BINARY_NAME} PRIVATE
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags

    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)