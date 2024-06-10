cmake_minimum_required(VERSION 3.22)

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.

project(OpenGLAppTutorial VERSION 1.0.0)

# Include JUCE directory (if it's included directly in your source tree)
# Uncomment if JUCE is included as a submodule or in your project directory
# add_subdirectory(JUCE)

# Add JUCE package if installed via package manager
# Uncomment if JUCE is installed globally
# find_package(JUCE CONFIG REQUIRED)

# Add the GUI application
juce_add_gui_app(OpenGLAppTutorial
        VERSION "1.0.0"
        COMPANY_NAME "JUCE"
        PRODUCT_NAME "OpenGL App Tutorial"
        COMPANY_WEBSITE "http://juce.com")

# Generate the JUCE header
juce_generate_juce_header(OpenGLAppTutorial)

# Add the source files
target_sources(OpenGLAppTutorial
        PRIVATE
        Source/Main.cpp
        Source/OpenGLAppTutorial.h)

juce_add_binary_data(BinaryData SOURCES
        ../assets/images/background.png
)

# Define preprocessor definitions
target_compile_definitions(OpenGLAppTutorial
        PRIVATE
        JUCE_WEB_BROWSER=0  # Disable JUCE web browser
        JUCE_USE_CURL=0     # Disable CURL support
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:OpenGLAppTutorial,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:OpenGLAppTutorial,JUCE_VERSION>")

# Link the required JUCE modules
target_link_libraries(OpenGLAppTutorial
        PRIVATE
        BinaryData
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
