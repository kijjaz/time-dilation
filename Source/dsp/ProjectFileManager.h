#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "RelativisticNodeGraph.h"

namespace time_dilation
{

class ProjectFileManager
{
public:
    static ProjectFileManager& getInstance();

    juce::File getTempCacheDirectory() const;
    juce::File getAudioAssetsDirectory (const juce::File& projectFolder) const;
    juce::File getMidiAssetsDirectory (const juce::File& projectFolder) const;

    bool saveProjectBundle (const juce::File& targetProjectFolder, RelativisticNodeGraph& graph);
    bool loadProjectBundle (const juce::File& projectFolderOrFile, RelativisticNodeGraph& graph);

    juce::File saveAudioClipToCache (const juce::AudioBuffer<float>& buffer, double sampleRate, const std::string& clipName);

private:
    ProjectFileManager();
    juce::File currentProjectFolder;
};

} // namespace time_dilation
