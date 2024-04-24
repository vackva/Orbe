cmake_minimum_required(VERSION 3.22)

project(BinauralPanner VERSION 0.0.1)
set (TARGET_NAME ${PROJECT_NAME})

if(APPLE)
    set(FORMATS_TO_BUILD AU VST3 Standalone)
else() # Maybe add LV2 here for Linux
    set(FORMATS_TO_BUILD VST3 Standalone)
endif()


# find_package(JUCE CONFIG REQUIRED)
add_subdirectory(modules/JUCE)


# Disable building tests for libmysofa
set(BUILD_TESTS OFF CACHE BOOL "Disable building tests" FORCE)
# Disable building shared libraries for libmysofa
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared instead of static libraries." FORCE)

if (MSVC)
    set(ZLIB_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/modules/libmysofa/windows/third-party/zlib-1.2.11/lib")
    set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/libmysofa/windows/third-party/zlib-1.2.11/include")
    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/modules/libmysofa/windows/third-party/zlib-1.2.11/lib")
    add_subdirectory(modules/libmysofa)
else()
    # Include the libmysofa project
    add_subdirectory(modules/libmysofa)
endif ()


juce_add_plugin(${TARGET_NAME}
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    # COMPANY_NAME ...                          # Specify the name of the plugin's author
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Juce               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Dem0                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS ${FORMATS_TO_BUILD}                 # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Binaural Panner"              # The name of the final executable, which can differ from the target name
)

if(APPLE AND CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    set_target_properties(${TARGET_NAME} PROPERTIES
            VST3_AUTO_MANIFEST FALSE
    )
endif()

juce_generate_juce_header(${TARGET_NAME})

target_sources(${TARGET_NAME}
    PRIVATE
        source/PluginEditor.cpp
        source/PluginProcessor.cpp
        source/PluginParameters.cpp

        source/ui/PannerVisualisation.cpp
        source/ui/BackgroundComponent.cpp
)

set(SOFA_TEST_FILE "${CMAKE_CURRENT_LIST_DIR}/assets/pp1_HRIRs_measured.sofa")

target_compile_definitions(${TARGET_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        SOFA_TEST_FILE_PATH="${SOFA_TEST_FILE}"
        JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
)

juce_add_binary_data(AudioPluginData SOURCES
    assets/pp1_HRIRs_measured.sofa
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        AudioPluginData
        juce::juce_audio_utils
        juce::juce_dsp
        mysofa-static 
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
