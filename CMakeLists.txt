cmake_minimum_required(VERSION 3.1.5)

set(PROJECT_NAME "ScaleRemapper")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(PROJECT_NAME "${PROJECT_NAME}Dev")
endif()

set(VERSION "0.6.1")

set(FORMATS "")

set(SRC_DIR "./src")
set(JUCE_DIR "external/juce")
set(REACT_JUCE "react_juce")
set(ROLI_BLOCKS "roli_blocks_basics")
set(BLOCKS_DIR "external/${ROLI_BLOCKS}")
set(REACT_JUCE_DIR "external/react-juce/${REACT_JUCE}")
set(JS_BUNDLE_PATH "./ui/build/js/bundle.js")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_PLUGIN_IS_STANDALONE)
    message(STATUS "Building as STANDALONE")
    list(APPEND FORMATS "Standalone")
else()
    message(STATUS "Building as Plugin")
    list(APPEND FORMATS "LV2")
    list(APPEND FORMATS "VST3")
endif()

project(${PROJECT_NAME} VERSION ${VERSION})

add_subdirectory(${JUCE_DIR} "juce")

juce_add_module(${BLOCKS_DIR})
juce_add_module(${REACT_JUCE_DIR})

if(WIN32 AND CMAKE_PLUGIN_IS_STANDALONE)
    message(STATUS "Building for Windows...")
    set(ASIO_DIR "sdk/asio")
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${ASIO_DIR})
        set(BUILD_ASIO 1)
    endif()
endif()

set(VST2_DIR "sdk/vst2")
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${VST2_DIR} AND NOT CMAKE_PLUGIN_IS_STANDALONE)
    set(BUILD_VST2 1)
    juce_set_vst2_sdk_path(${VST2_DIR})
    list(APPEND FORMATS "VST")
endif()

target_compile_definitions(${REACT_JUCE} INTERFACE REACTJUCE_USE_DUKTAPE=1)

message(STATUS "Formats added: ${FORMATS}")

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME XivilaY
    PRODUCT_NAME "${PROJECT_NAME}"
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    PLUGIN_MANUFACTURER_CODE XivY
    PLUGIN_CODE Xiv8
    LV2URI "https://github.com/xivilay/scale-remapper"
    VST3_CATEGORIES "Instrument Tools"
    ICON_SMALL "${SRC_DIR}/icons/small.png"
    FORMATS ${FORMATS}
)

juce_add_binary_data(BinaryData SOURCES
    ${JS_BUNDLE_PATH}
)

set_target_properties(BinaryData PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

juce_generate_juce_header(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    "${SRC_DIR}/Main.cpp"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
)

if(WIN32 AND NOT CMAKE_PLUGIN_IS_STANDALONE) 
    target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_USE_WINRT_MIDI=1) 
endif()

if(BUILD_VST2)
    message(STATUS "Adding VST2...")
    target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_VST3_CAN_REPLACE_VST2=1)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_VST3_CAN_REPLACE_VST2=0)
endif(BUILD_VST2)

if(BUILD_ASIO)
    message(STATUS "Adding ASIO...")
    target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_ASIO=1)
    target_include_directories(${PROJECT_NAME} PRIVATE ${ASIO_DIR})
endif(BUILD_ASIO)

target_link_libraries(${PROJECT_NAME} PRIVATE
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

    ${ROLI_BLOCKS}
    ${REACT_JUCE}
    BinaryData
)