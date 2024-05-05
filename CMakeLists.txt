cmake_minimum_required(VERSION 3.22)

cmake_policy(SET CMP0048 NEW)

project(BinauralPanner VERSION 0.0.1)
set (TARGET_NAME ${PROJECT_NAME})

if(APPLE)
    set(FORMATS_TO_BUILD AU VST3 Standalone)
else() # Maybe add LV2 here for Linux
    set(FORMATS_TO_BUILD VST3 Standalone)
endif()


# find_package(JUCE CONFIG REQUIRED)
add_subdirectory(modules/JUCE)

set(BUILD_TESTS OFF CACHE BOOL "Disable building tests" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared instead of static libraries." FORCE)
add_subdirectory(modules/customlibmysofa SYSTEM)

if(UNIX AND NOT APPLE)
    set_target_properties(mysofa-static PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

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
    VST3_AUTO_MANIFEST FALSE

    PLUGIN_MANUFACTURER_CODE Juce               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Dem0                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS ${FORMATS_TO_BUILD}                 # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Binaural Panner"              # The name of the final executable, which can differ from the target name
)

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


# Disable all wanrings from mysofa
if(MSVC)
    set_target_properties(mysofa-static PROPERTIES COMPILE_OPTIONS "/W0")
else()
    set_target_properties(mysofa-static PROPERTIES COMPILE_OPTIONS "-w")
endif()
