cmake_minimum_required(VERSION 3.22)
project(DPxADJU_AI VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(DPX_BUILD_JUCE "Build JUCE VST3 and Standalone targets" OFF)
option(DPX_BUILD_TESTS "Build DSP tests" ON)
add_library(dpx_dsp STATIC "Source Code/Core/DspEngine.cpp")
target_include_directories(dpx_dsp PUBLIC "Source Code/Core")
target_compile_features(dpx_dsp PUBLIC cxx_std_17)
if(MSVC)
  target_compile_options(dpx_dsp PRIVATE /W4 /permissive-)
else()
  target_compile_options(dpx_dsp PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(DPX_BUILD_TESTS)
  enable_testing()
  add_executable(dpx_dsp_tests Tests/DspTests.cpp)
  target_link_libraries(dpx_dsp_tests PRIVATE dpx_dsp)
  add_test(NAME dpx_dsp_tests COMMAND dpx_dsp_tests)
endif()
if(DPX_BUILD_JUCE)
  if(NOT DEFINED JUCE_SOURCE_DIR)
    message(FATAL_ERROR "Set JUCE_SOURCE_DIR to a local JUCE checkout (JUCE 7/8).")
  endif()
  add_subdirectory(${JUCE_SOURCE_DIR} JUCE)
  juce_add_plugin(DPxADJU_AI
    COMPANY_NAME "DPxADJU"
    BUNDLE_ID "com.dpxadju.ai.mastering"
    PLUGIN_MANUFACTURER_CODE DpAj
    PLUGIN_CODE DpA1
    FORMATS VST3 Standalone
    PRODUCT_NAME "DPxADJU AI"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE COPY_PLUGIN_AFTER_BUILD FALSE)
  target_sources(DPxADJU_AI PRIVATE "Source Code/Plugin/PluginProcessor.cpp" "Source Code/Plugin/PluginEditor.cpp" "Source Code/Plugin/PluginProcessor.h" "Source Code/Plugin/PluginEditor.h" "Source Code/Plugin/WavesScanner.cpp" "Source Code/Plugin/WavesScanner.h")
  target_compile_definitions(DPxADJU_AI PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(DPxADJU_AI PRIVATE dpx_dsp juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp)
endif()
if(DPX_BUILD_JUCE AND WIN32)
  if(TARGET DPxADJU_AI_Standalone)
    add_custom_command(TARGET DPxADJU_AI_Standalone POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:DPxADJU_AI_Standalone>" "$<TARGET_FILE_DIR:DPxADJU_AI_Standalone>/DPxADJU AI Studio.exe")
  endif()
endif()
