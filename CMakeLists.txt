cmake_minimum_required(VERSION 3.22)

project(TimeDilationDAW VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch Tracktion Engine & JUCE
message(STATUS "Fetching Tracktion Engine...")
FetchContent_Declare(
    tracktion_engine
    GIT_REPOSITORY https://github.com/Tracktion/tracktion_engine.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(tracktion_engine)

# Define Native Standalone DAW App & VST3/AU Plugin
juce_add_plugin(TimeDilationDAW
    COMPANY_NAME "Kijjaz"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Kjjz
    PLUGIN_CODE Tmdl
    FORMATS Standalone VST3 AU
    PRODUCT_NAME "Time Dilation DAW"
)

# Source Files
target_sources(TimeDilationDAW PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/dsp/HermiteResampler.h
    Source/dsp/TimeDilationEngine.h
    Source/dsp/TimeDilationEngine.cpp
    Source/dsp/GranularEngine.h
    Source/dsp/GranularEngine.cpp
    Source/dsp/TidalBeatEngine.h
    Source/dsp/TidalBeatEngine.cpp
    Source/dsp/RelativisticNodeGraph.h
    Source/dsp/RelativisticNodeGraph.cpp
    Source/dsp/RelativisticNodeObjects.h
    Source/dsp/RelativisticNodeObjects.cpp
    Source/dsp/RelativisticExpressionParser.h
    Source/dsp/RelativisticExpressionParser.cpp
    Source/gui/CarbonGoldLookAndFeel.h
    Source/gui/CarbonGoldLookAndFeel.cpp
    Source/gui/AudioSettingsComponent.h
    Source/gui/AudioSettingsComponent.cpp
    Source/gui/ModularDataflowCanvas.h
    Source/gui/ModularDataflowCanvas.cpp
    Source/gui/RelativisticStudioComponent.h
    Source/gui/RelativisticStudioComponent.cpp
    Source/gui/PianoRollComponent.h
    Source/gui/PianoRollComponent.cpp
    Source/gui/FullStudioWorkstation.h
    Source/gui/FullStudioWorkstation.cpp
    Source/gui/MinimalistStudioComponent.h
    Source/gui/MinimalistStudioComponent.cpp
    Source/gui/RelativisticDAWWorkstation.h
    Source/gui/RelativisticDAWWorkstation.cpp
    Source/gui/TimelineClipComponent.h
    Source/gui/TimelineClipComponent.cpp
    Source/gui/TactileAudioStudioComponent.h
    Source/gui/TactileAudioStudioComponent.cpp
    Source/gui/AnsiBoxStudioComponent.h
    Source/gui/AnsiBoxStudioComponent.cpp
    Source/gui/GranularStudioComponent.h
    Source/gui/GranularStudioComponent.cpp
    Source/gui/TidalBeatStudioComponent.h
    Source/gui/TidalBeatStudioComponent.cpp
    Source/gui/RelativisticCanvasComponent.h
    Source/gui/RelativisticCanvasComponent.cpp
)

# Include Directories
target_include_directories(TimeDilationDAW PRIVATE
    Source
    Source/dsp
    Source/gui
)

# Binary Assets (Fonts & Graphics)
juce_add_binary_data(TimeDilationAssets
    SOURCES
        Source/assets/fonts/SmoochSans.ttf
        Source/assets/fonts/NotoSans.ttf
)

# Link Tracktion Engine & JUCE Modules
target_link_libraries(TimeDilationDAW PRIVATE
    TimeDilationAssets
    tracktion_engine
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# Compiler Optimizations & Flags
if (MSVC)
    target_compile_options(TimeDilationDAW PRIVATE /W4 /fp:fast)
else()
    target_compile_options(TimeDilationDAW PRIVATE -Wall -Wextra -ffast-math -O3)
endif()

juce_generate_juce_header(TimeDilationDAW)
