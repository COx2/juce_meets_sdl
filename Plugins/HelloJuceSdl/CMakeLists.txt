project(HelloJuceSdl VERSION 0.0.1)

set (BaseTargetName HelloJuceSdl)

# Adds all the module sources so they appear correctly in the IDE
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# Set any of these to "ON" if you want to build one of the juce examples or extras (Projucer/AudioPluginHost, etc):
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)

message(STATUS "THIS_JUCE_DIRECTORY: ${THIS_JUCE_DIRECTORY}")
add_subdirectory(${THIS_JUCE_DIRECTORY} JUCE EXCLUDE_FROM_ALL)

message(STATUS "THIS_SDL_DIRECTORY: ${THIS_SDL_DIRECTORY}")
add_subdirectory(${THIS_SDL_DIRECTORY} SDL EXCLUDE_FROM_ALL)

juce_add_plugin("${BaseTargetName}"
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME "COx2"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Cox2
    PLUGIN_CODE Jsdl
    FORMATS Standalone VST3 AU)

juce_generate_juce_header(${BaseTargetName})

target_sources(${BaseTargetName}
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/SDLRunner.cpp
        Source/SDLRunner.h)

target_compile_definitions(${BaseTargetName}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

# Use PUBLIC to effect linked target like VST3, Standalone, CLAP, ...
target_compile_features(${BaseTargetName}
    PUBLIC
        cxx_std_17)

target_link_libraries(${BaseTargetName}
    PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_audio_devices
        SDL2::SDL2-static
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

message(STATUS "[${BaseTargetName}] Binary directory: ${CMAKE_CURRENT_BINARY_DIR}")
