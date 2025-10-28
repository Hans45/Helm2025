cmake_minimum_required(VERSION 3.15)

# Set a default build type if none was specified
set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

project(Helm2025
  VERSION 0.10.0
  DESCRIPTION "A cross-platform polyphonic synthesizer, built by Hans45 from the original Helm project by Matt Tytel."
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable SIMD optimizations
include(CheckCXXCompilerFlag)
if(MSVC)
    check_cxx_compiler_flag("/arch:AVX2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        add_compile_options("/arch:AVX2")
    endif()
else()
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        add_compile_options("-mavx2")
    endif()
endif()

# It's "bad form" to set this globally, but it's convenient.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# VST3 SDK Handling
set(JUCE_VST3_SDK "${CMAKE_CURRENT_SOURCE_DIR}/external/VST3_SDK")

# ==================================================

add_subdirectory(concurrentqueue)
add_subdirectory(mopo)
add_subdirectory(external/JUCE)

juce_add_binary_data(HelmData SOURCES
  fonts/DroidSansMono.ttf
  fonts/Roboto-Regular.ttf
  fonts/Roboto-Light.ttf
  fonts/Roboto-Thin.ttf
  images/helm2025_icon_32_1x.png
  images/helm2025_icon_32_2x.png
  images/helm2025_icon_128_1x.png
  images/helm2025_icon_128_2x.png
  images/helm2025_icon_256_1x.png
  images/helm2025_icon_256_2x.png
  images/helm2025_icon_512_1x.png
  images/helm2025_icon_512_2x.png
  images/modulation_selected_active.png
  images/modulation_selected_inactive.png
  images/modulation_unselected_active.png
  images/modulation_unselected_inactive.png)

# ==================================================

juce_add_plugin(Helm2025Plugin
  ICON_BIG images/helm2025_icon_512_2x.png
  ICON_SMALL images/helm2025_icon_128_2x.png

  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE

  PRODUCT_NAME "Helm2025"
  COMPANY_NAME "Hans45"
  BUNDLE_ID "com.hans45.helm2025"
  PLUGIN_MANUFACTURER_CODE "Tyte"
  FORMATS VST3 AU LV2

  LV2URI "https://github.com/Hans45/helm2025"
  LV2_WANTS_LATENCY FALSE)

juce_generate_juce_header(Helm2025Plugin)

target_sources(Helm2025Plugin PRIVATE
  src/synthesis/helm2025_engine_simd.h
  src/plugin/helm2025_plugin_simd.h
  src/common/border_bounds_constrainer.cpp
  src/common/file_list_box_model.cpp
  src/common/helm2025_common.cpp
  src/common/load_save.cpp
  src/common/midi_manager.cpp
  src/common/startup.cpp
  src/common/synth_base.cpp
  src/common/synth_gui_interface.cpp
  src/editor_components/bpm_slider.cpp
  src/editor_components/filter_response.cpp
  src/editor_components/filter_selector.cpp
  src/editor_components/global_tool_tip.cpp
  src/editor_components/graphical_envelope.cpp
  src/editor_components/graphical_step_sequencer.cpp
  src/editor_components/midi_keyboard.cpp
  src/editor_components/modulation_button.cpp
  src/editor_components/modulation_highlight.cpp
  src/editor_components/modulation_meter.cpp
  src/editor_components/modulation_slider.cpp
  src/editor_components/open_gl_background.cpp
  src/editor_components/open_gl_component.cpp
  src/editor_components/open_gl_envelope.cpp
  src/editor_components/open_gl_modulation_meter.cpp
  src/editor_components/open_gl_oscilloscope.cpp
  src/editor_components/open_gl_peak_meter.cpp
  src/editor_components/open_gl_wave_viewer.cpp
  src/editor_components/oscilloscope.cpp
  src/editor_components/patch_selector.cpp
  src/editor_components/retrigger_selector.cpp
  src/editor_components/synth_button.cpp
  src/editor_components/synth_slider.cpp
  src/editor_components/tempo_selector.cpp
  src/editor_components/text_selector.cpp
  src/editor_components/text_slider.cpp
  src/editor_components/value_entry_component.cpp
  src/editor_components/wave_selector.cpp
  src/editor_components/wave_viewer.cpp
  src/editor_components/xy_pad.cpp
  src/editor_sections/about_section.cpp
  src/editor_sections/arp_section.cpp
  src/editor_sections/bpm_section.cpp
  src/editor_sections/contribute_section.cpp
  src/editor_sections/delay_section.cpp
  src/editor_sections/delete_section.cpp
  src/editor_sections/distortion_section.cpp
  src/editor_sections/dynamic_section.cpp
  src/editor_sections/envelope_section.cpp
  src/editor_sections/extra_mod_section.cpp
  src/editor_sections/feedback_section.cpp
  src/editor_sections/filter_section.cpp
  src/editor_sections/formant_section.cpp
  src/editor_sections/full_interface.cpp
  src/editor_sections/lfo_section.cpp
  src/editor_sections/mixer_section.cpp
  src/editor_sections/modulation_manager.cpp
  src/editor_sections/noise_section.cpp
  src/editor_sections/open_gl_modulation_manager.cpp
  src/editor_sections/oscillator_section.cpp
  src/editor_sections/patch_browser.cpp
  src/editor_sections/reverb_section.cpp
  src/editor_sections/save_section.cpp
  src/editor_sections/step_sequencer_section.cpp
  src/editor_sections/stutter_section.cpp
  src/editor_sections/sub_section.cpp
  src/editor_sections/synthesis_interface.cpp
  src/editor_sections/synth_section.cpp
  src/editor_sections/update_check_section.cpp
  src/editor_sections/voice_section.cpp
  src/editor_sections/volume_section.cpp
  src/look_and_feel/browser_look_and_feel.cpp
  src/look_and_feel/colors.cpp
  src/look_and_feel/default_look_and_feel.cpp
  src/look_and_feel/fonts.cpp
  src/look_and_feel/modulation_look_and_feel.cpp
  src/look_and_feel/shaders.cpp
  src/look_and_feel/text_look_and_feel.cpp
  src/plugin/helm2025_editor.cpp
  src/plugin/helm2025_plugin.cpp
  src/synthesis/dc_filter.cpp
  src/synthesis/detune_lookup.cpp
  src/synthesis/fixed_point_oscillator.cpp
  src/synthesis/fixed_point_wave.cpp
  src/synthesis/gate.cpp
  src/synthesis/helm2025_engine.cpp
  src/synthesis/helm2025_midi_handler.cpp
  src/synthesis/helm2025_lfo.cpp
  src/synthesis/helm2025_module.cpp
  src/synthesis/helm2025_oscillators.cpp
  src/synthesis/helm2025_voice_handler.cpp
  src/synthesis/noise_oscillator.cpp
  src/synthesis/peak_meter.cpp
  src/synthesis/resonance_cancel.cpp
  src/synthesis/trigger_random.cpp
  src/synthesis/value_switch.cpp)

target_include_directories(Helm2025Plugin PUBLIC
  src/common
  src/editor_components
  src/editor_sections
  src/look_and_feel
  src/plugin
  src/synthesis)

target_compile_definitions(Helm2025Plugin PUBLIC
  # Helm tries to detect at runtime whether it's actually installed,
  # whether it's just being queried for its factory presets by
  # the LV2 .ttl generator. If it's the latter, Helm needs to know
  # where its factory presets are on the developer's disk. Not ideal
  # for a number of reasons, but it'll have to do for now.
  HELM2025_LV2GEN_FACTORY_PRESET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/patches/Factory Presets"

  # Enable SIMD optimizations
  MOPO_USE_SIMD=1

  # Enable other optimizations
  MOPO_USE_CACHE=1
  MOPO_ALIGNED_MALLOC=1

  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_USE_XRANDR=0)

target_link_libraries(Helm2025Plugin
  PUBLIC
    HelmData
    concurrentqueue
    mopo

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
    juce::juce_opengl

    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    # juce::juce_recommended_warning_flags
    )

# ==================================================

juce_add_gui_app(Helm2025Standalone
  ICON_BIG images/helm2025_icon_512_2x.png
  ICON_SMALL images/helm2025_icon_128_2x.png

  PRODUCT_NAME "Helm2025"
  COMPANY_NAME "Hans45"
  BUNDLE_ID "com.hans45.helm2025.standalone")

juce_generate_juce_header(Helm2025Standalone)

target_sources(Helm2025Standalone PRIVATE
  src/standalone/helm2025_editor.cpp
  src/standalone/helm2025_computer_keyboard.cpp
  src/standalone/main.cpp)

target_include_directories(Helm2025Standalone PRIVATE
  src/standalone)

# Define a user-configurable path for the ASIO SDK
set(HELM2025_ASIO_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/ASIOSDK" CACHE PATH "Path to the Steinberg ASIO SDK")

# Conditionally enable ASIO support
if(EXISTS "${HELM2025_ASIO_SDK_PATH}/common/asiosys.h")
    message(STATUS "ASIO SDK found at ${HELM2025_ASIO_SDK_PATH}. Enabling ASIO support.")
    target_include_directories(Helm2025Standalone PRIVATE
        "${HELM2025_ASIO_SDK_PATH}/common"
        "${HELM2025_ASIO_SDK_PATH}/host")
    target_compile_definitions(Helm2025Standalone PRIVATE
        JUCE_ASIO=1
        JUCE_ASIO_SDK="${HELM2025_ASIO_SDK_PATH}")
else()
    message(STATUS "ASIO SDK not found. Building without ASIO support. (Set HELM2025_ASIO_SDK_PATH to enable)")
    target_compile_definitions(Helm2025Standalone PRIVATE JUCE_ASIO=0)
endif()

target_compile_definitions(Helm2025Standalone PRIVATE
  JUCE_WASAPI=1
  JUCE_DIRECTSOUND=1
  JUCE_JACK=0
  JUCE_ALSA=0
  JUCE_JACK_CLIENT_NAME="Helm2025"
  JUCE_ALSA_MIDI_NAME="Helm2025")

target_link_libraries(Helm2025Standalone PRIVATE Helm2025Plugin)

# Unit tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  add_executable(parameter_interpolator_test
    tests/test_parameter_interpolator.cpp)

  target_include_directories(parameter_interpolator_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/synthesis)

  target_compile_features(parameter_interpolator_test PRIVATE cxx_std_17)

  # Link nothing heavy; test only includes header-only code
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  add_executable(parameter_batch_interpolator_test
    tests/test_parameter_batch_interpolator.cpp)

  target_include_directories(parameter_batch_interpolator_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/synthesis)

  target_compile_features(parameter_batch_interpolator_test PRIVATE cxx_std_17)
endif()