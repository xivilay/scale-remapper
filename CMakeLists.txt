cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME "ScaleRemapper")

if(ANDROID_PLATFORM)
    set(PROJECT_NAME "juce_jni")
else()
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(PROJECT_NAME "${PROJECT_NAME}Dev")
    endif()
endif()

set(VERSION "0.7.5")

set(FORMATS "Standalone" "VST3" "AUv3" "AU" "LV2")

set(SRC_DIR "./src")
set(JUCE_DIR "external/juce")
set(REACT_JUCE "react_juce")
set(ROLI_BLOCKS "roli_blocks_basics")
set(BLOCKS_DIR "external/${ROLI_BLOCKS}")
set(REACT_JUCE_DIR "external/react-juce/${REACT_JUCE}")
set(JS_BUNDLE_PATH "./ui/build/js/bundle.js")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(${PROJECT_NAME} VERSION ${VERSION})

add_subdirectory(${JUCE_DIR} "juce")

juce_add_module(${BLOCKS_DIR})
juce_add_module(${REACT_JUCE_DIR})

if(WIN32)
    message(STATUS "Building for Windows...")
    set(ASIO_DIR "sdk/asio")
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${ASIO_DIR})
        set(BUILD_ASIO 1)
    endif()
endif()

set(VST2_DIR "sdk/vst2")
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${VST2_DIR})
    set(BUILD_VST2 1)
    juce_set_vst2_sdk_path(${VST2_DIR})
    list(APPEND FORMATS "VST")
endif()

target_compile_definitions(${REACT_JUCE} INTERFACE REACTJUCE_USE_DUKTAPE=1)

message(STATUS "Formats added: ${FORMATS}")

if(ANDROID_PLATFORM)
    add_library(${PROJECT_NAME} SHARED)
    _juce_initialise_target(${PROJECT_NAME})
else()
    juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME XivilaY
        PRODUCT_NAME "${PROJECT_NAME}"
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT TRUE
        PLUGIN_MANUFACTURER_CODE XivY
        PLUGIN_CODE Xiv8
        LV2URI "https://github.com/xivilay/scale-remapper"
        VST3_CATEGORIES "Instrument Tools"
        ICON_SMALL "${SRC_DIR}/icons/small.png"
        FORMATS ${FORMATS}
        AU_SANDBOX_SAFE TRUE  
        BUNDLE_ID "dev.xivilay.scaleRemapper"
        BLUETOOTH_PERMISSION_ENABLED TRUE 
        BLUETOOTH_PERMISSION_TEXT "Needed for bluetooth midi input devices"
    )
endif()

juce_add_binary_data(BinaryData SOURCES
    ${JS_BUNDLE_PATH}
)

set_target_properties(BinaryData PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

juce_generate_juce_header(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    "${SRC_DIR}/Main.cpp"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    JucePlugin_Build_Standalone=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
    ) 
    add_link_options("-Wl,-weak_reference_mismatches,weak")
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

if (NOT ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        juce::juce_audio_plugin_client
    )
endif()

if(ANDROID)
    message(STATUS "Building for ANDROID...")

    string(REPLACE "\\" "/" ANDROID_NDK ${ANDROID_NDK})

    set(CPU_FEATURES_LIB "cpufeatures")
    set(CPU_FEATURES_ENTRY "sources/android/cpufeatures/cpu-features.c")

    include_directories(AFTER
        "${JUCE_MODULES_DIR}"
        "${JUCE_MODULES_DIR}/juce_audio_processors/format_types/VST3_SDK"
        "${ANDROID_NDK}/sources/android/cpufeatures"
    )

    add_library("${CPU_FEATURES_LIB}" STATIC "${ANDROID_NDK}/${CPU_FEATURES_ENTRY}")
    set_source_files_properties("${ANDROID_NDK}/${CPU_FEATURES_ENTRY}" PROPERTIES COMPILE_FLAGS "-Wno-sign-conversion -Wno-gnu-statement-expression")

    add_definitions(
        "-DJUCER_ANDROIDSTUDIO_7F0E4A25=1"
        "-DJUCE_ANDROID=1"
        "-DJUCE_ANDROID_API_VERSION=24"
        "-DJUCE_PUSH_NOTIFICATIONS=1"
        "-DJUCE_PUSH_NOTIFICATIONS_ACTIVITY=\"com/rmsl/juce/JuceActivity\""
        "-DJUCE_APP_VERSION=${VERSION}"
        "-DJucePlugin_Build_Standalone=1"
        "-DJucePlugin_Name=\"${PROJECT_NAME}\""
        "-DJucePlugin_Desc=\"${PROJECT_NAME}\""
        "-DJucePlugin_Manufacturer=\"XivilaY\""
        "-DJucePlugin_ManufacturerWebsite=\"xivilay.dev\""
        "-DJucePlugin_ManufacturerEmail=\"\""
        "-DJucePlugin_ManufacturerCode=XivY"
        "-DJucePlugin_PluginCode=Xiv8"
        "-DJucePlugin_VSTUniqueID=\"Xiv8\""
        "-DJucePlugin_IsSynth=0"
        "-DJucePlugin_WantsMidiInput=1"
        "-DJucePlugin_ProducesMidiOutput=1"
        "-DJucePlugin_IsMidiEffect=1"
        "-DJucePlugin_EditorRequiresKeyboardFocus=1"
        "-DJucePlugin_Version=${VERSION}"
        "-DJucePlugin_VersionString=\"${VERSION}\""
    )

    if(JUCE_BUILD_CONFIGURATION MATCHES "DEBUG")
        add_definitions("-DDEBUG=1" "-D_DEBUG=1")
    elseif(JUCE_BUILD_CONFIGURATION MATCHES "RELEASE")
        add_definitions("-DNDEBUG=1")
        if(NOT (ANDROID_ABI STREQUAL "mips" OR ANDROID_ABI STREQUAL "mips64"))
            set(CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE} -flto")
            set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -flto")
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
        endif()
    else()
        message(FATAL_ERROR "No matching build-configuration found.")
    endif()

    enable_language(ASM)

    target_compile_options(${PROJECT_NAME} PRIVATE "-fsigned-char")

    find_library(log "log")
    find_library(android "android")
    find_library(glesv3 "GLESv3")
    find_library(egl "EGL")

    add_library("juceAndroid" INTERFACE)

    target_link_libraries("juceAndroid" INTERFACE
        ${log}
        ${android}
        ${glesv3}
        ${egl}
        ${CPU_FEATURES_LIB}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        juce::juce_audio_plugin_client_Standalone
        "juceAndroid"
    )
endif(ANDROID)
