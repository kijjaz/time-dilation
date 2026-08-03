cmake_minimum_required(VERSION 3.22)

project(TimeDilationDAW VERSION 0.0.1 LANGUAGES C CXX)

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

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(TRACKTION_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(TRACKTION_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(tracktion_engine)

# Binary Assets (Fonts & Graphics)
juce_add_binary_data(TimeDilationAssets
    SOURCES
        Source/assets/fonts/Oxanium-Bold.ttf
        Source/assets/fonts/Oxanium-Regular.ttf
        Source/assets/fonts/SmoochSans.ttf
        Source/assets/fonts/NotoSans.ttf
        Source/assets/logo_banner.svg
        Source/assets/logo_banner.svg.png
        Source/assets/logo_icon.svg.png
)

# Standalone Application Target
juce_add_gui_app(TimeDilationDAW_App
    COMPANY_NAME "Kijjaz"
    VERSION "0.0.1"
    PRODUCT_NAME "Time Dilation DAW"
    ICON_BIG "Source/assets/logo_icon.svg.png"
    ICON_SMALL "Source/assets/logo_icon.svg.png"
)

target_sources(TimeDilationDAW_App PRIVATE
    Source/Main.cpp
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
    Source/dsp/RelativisticTimeline.h
    Source/dsp/RelativisticTimeline.cpp
    Source/gui/TimelineEditorComponent.h
    Source/gui/TimelineEditorComponent.cpp
    Source/dsp/ProjectFileManager.h
    Source/dsp/ProjectFileManager.cpp
    Source/dsp/RelativisticSequencers.h
    Source/dsp/RelativisticSequencers.cpp
)

target_include_directories(TimeDilationDAW_App PRIVATE
    Source
    Source/dsp
    Source/gui
)

target_link_libraries(TimeDilationDAW_App PRIVATE
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

if (MSVC)
    target_compile_options(TimeDilationDAW_App PRIVATE /W4 /fp:fast)
else()
    target_compile_options(TimeDilationDAW_App PRIVATE -Wall -Wextra -ffast-math 