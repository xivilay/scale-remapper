cmake_minimum_required(VERSION 3.1.5)

set(BINARY_NAME "ScaleRemapper")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(BINARY_NAME "${BINARY_NAME}Dev")
endif()

set(VERSION "0.3.0")
set(JUCE_DIR "external/juce")
set(SRC_DIR "./src")

set(REACT_JUCE "react_juce")
# see https://github.com/nick-thompson/react-juce/tree/master/react_juce
set(REACT_JUCE_DIR "external/react-juce/${REACT_JUCE}")  # path to REACT JUCE
set(JS_BUNDLE_PATH "./ui/build/js/bundle.js")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(${BINARY_NAME} VERSION ${VERSION})

add_subdirectory(${JUCE_DIR} "juce")

juce_add_module(${REACT_JUCE_DIR})

target_compile_definitions(${REACT_JUCE} INTERFACE REACTJUCE_USE_DUKTAPE=1)

juce_add_plugin(${BINARY_NAME}
    COMPANY_NAME XivilaY
    PRODUCT_NAME "${BINARY_NAME}"
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    PLUGIN_MANUFACTURER_CODE XivY
    PLUGIN_CODE Xiv8
    FORMATS VST3
)

juce_add_binary_data(BinaryData SOURCES
    ${JS_BUNDLE_PATH}
)

set_target_properties(BinaryData PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

juce_generate_juce_header(${BINARY_NAME})

target_sources(${BINARY_NAME} PRIVATE
    "${SRC_DIR}/Main.cpp"
)

target_compile_definitions(${BINARY_NAME} PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    P_WIDTH=550
    P_HEIGHT=500
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

    ${REACT_JUCE}
    BinaryData
)