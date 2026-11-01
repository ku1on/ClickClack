cmake_minimum_required(VERSION 3.22)

project(ClickClack VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# macOS ARM64 Apple Silicon configuration
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build for Apple Silicon" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

# Find Homebrew packages
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# Include JUCE
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/JUCE/CMakeLists.txt")
    add_subdirectory(deps/JUCE)
else()
    message(FATAL_ERROR "JUCE not found in deps/JUCE. Please download JUCE into deps/JUCE.")
endif()

# Find required libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSAMPLERATE REQUIRED samplerate)
pkg_check_modules(FFTW REQUIRED fftw3 fftw3f)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(EIGEN3 REQUIRED eigen3)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswresample)

# Essentia locations
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/essentia/lib/libessentia.a")
    set(ESSENTIA_STATIC_LIB "${CMAKE_CURRENT_SOURCE_DIR}/deps/essentia/lib/libessentia.a")
    set(ESSENTIA_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/essentia/include")
    set(ESSENTIA_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/essentia/include")
else()
    set(ESSENTIA_SRC_DIR "/tmp/clickclack_deps/essentia/src")
    set(ESSENTIA_BUILD_DIR "/tmp/clickclack_deps/essentia/build")
    set(ESSENTIA_STATIC_LIB "${ESSENTIA_BUILD_DIR}/src/libessentia.a")
endif()

link_directories(
    "/opt/homebrew/lib"
    ${LIBSAMPLERATE_LIBRARY_DIRS}
    ${FFTW_LIBRARY_DIRS}
    ${LIBYAML_LIBRARY_DIRS}
    ${TAGLIB_LIBRARY_DIRS}
    ${FFMPEG_LIBRARY_DIRS}
)

# Define JUCE Plugin
juce_add_plugin(ClickClack
    COMPANY_NAME "ClickClack Audio"
    BUNDLE_ID "com.clickclack.audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE "KdEo"
    PLUGIN_CODE "CkCk"
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "ClickClack"
    DESCRIPTION "Professional Real-Time BPM & Key Detector"
)

# Source files
target_sources(ClickClack PRIVATE
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/Audio/AudioInputProcessor.h
    Source/Audio/AudioInputProcessor.cpp
    Source/Audio/AudioBufferManager.h
    Source/Audio/AudioBufferManager.cpp
    Source/Analysis/AnalysisResult.h
    Source/Analysis/ConfidenceEstimator.h
    Source/Analysis/ConfidenceEstimator.cpp
    Source/Analysis/AnalysisWorker.h
    Source/Analysis/AnalysisWorker.cpp
    Source/Analysis/AnalysisEngine.h
    Source/Analysis/AnalysisEngine.cpp
    Source/DSP/CamelotMapper.h
    Source/DSP/CamelotMapper.cpp
    Source/DSP/BPMDetector.h
    Source/DSP/BPMDetector.cpp
    Source/DSP/KeyDetector.h
    Source/DSP/KeyDetector.cpp
    Source/UI/Theme/BrutalistTheme.h
    Source/UI/Components/BrutalistButton.h
    Source/UI/Components/BrutalistButton.cpp
    Source/UI/Components/ConfidenceBar.h
    Source/UI/Components/ConfidenceBar.cpp
    Source/UI/Components/StatusBadge.h
    Source/UI/Components/StatusBadge.cpp
    Source/UI/Components/AudioSignalMeter.h
    Source/UI/Components/AudioSignalMeter.cpp
    Source/Utils/LockFreeFifo.h
)

# Include directories
target_include_directories(ClickClack PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${ESSENTIA_SRC_DIR}"
    "${ESSENTIA_BUILD_DIR}/src"
    ${LIBSAMPLERATE_INCLUDE_DIRS}
    ${FFTW_INCLUDE_DIRS}
    ${LIBYAML_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)

# Compile definitions
target_compile_definitions(ClickClack PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    __STDC_CONSTANT_MACROS
)

# Link libraries
target_link_libraries(ClickClack PRIVATE
    "${ESSENTIA_STATIC_LIB}"
    /opt/homebrew/lib/libsamplerate.dylib
    /opt/homebrew/lib/libfftw3.dylib
    /opt/homebrew/lib/libfftw3f.dylib
    /opt/homebrew/lib/libyaml.dylib
    /opt/homebrew/lib/libtag.dylib
    ${FFMPEG_LIBRARIES}
    "-framework Accelerate"
    "-framework AudioToolbox"
    "-framework CoreAudio"
    "-framework CoreMIDI"
    "-framework Cocoa"
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp
    juce::juce_data_structures
    juce::juce_events
    juce::juce_core
)

target_link_directories(ClickClack PRIVATE
    ${LIBSAMPLERATE_LIBRARY_DIRS}
    ${FFTW_LIBRARY_DIRS}
    ${LIBYAML_LIBRARY_DIRS}
    ${TAGLIB_LIBRARY_DIRS}
    ${FFMPEG_LIBRARY_DIRS}
)
