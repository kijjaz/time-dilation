#include "ProjectFileManager.h"

namespace time_dilation
{

ProjectFileManager& ProjectFileManager::getInstance()
{
    static ProjectFileManager instance;
    return instance;
}

ProjectFileManager::ProjectFileManager()
{
    getTempCacheDirectory().createDirectory();
}

juce::File ProjectFileManager::getTempCacheDirectory() const
{
    auto userAppData = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
    return userAppData.getChildFile ("TimeDilationDAW").getChildFile ("Cache");
}

juce::File ProjectFileManager::getAudioAssetsDirectory (const juce::File& projectFolder) const
{
    return projectFolder.getChildFile ("Assets").getChildFile ("Audio");
}

juce::File ProjectFileManager::getMidiAssetsDirectory (const juce::File& projectFolder) const
{
    return projectFolder.getChildFile ("Assets").getChildFile ("MIDI");
}

juce::File ProjectFileManager::saveAudioClipToCache (const juce::AudioBuffer<float>& buffer, double sampleRate, const std::string& clipName)
{
    auto cacheDir = getTempCacheDirectory();
    cacheDir.createDirectory();

    juce::File outFile = cacheDir.getChildFile (juce::File::createLegalFileName (clipName) + "_" + juce::String (juce::Random::getSystemRandom().nextInt (9999)) + ".wav");

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer (wavFormat.createWriterFor (
        outFile.createOutputStream().release(),
        sampleRate,
        static_cast<unsigned int>(buffer.getNumChannels()),
        16,
        {},
        0
    ));

    if (writer != nullptr)
    {
        writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
    }

    return outFile;
}

bool ProjectFileManager::saveProjectBundle (const juce::File& targetProjectFileOrFolder, RelativisticNodeGraph& graph)
{
    juce::File mainFile;
    if (targetProjectFileOrFolder.isDirectory())
    {
        currentProjectFolder = targetProjectFileOrFolder;
        mainFile = targetProjectFileOrFolder.getChildFile ("patch.patch");
    }
    else
    {
        currentProjectFolder = targetProjectFileOrFolder.getParentDirectory();
        mainFile = targetProjectFileOrFolder;
    }

    currentProjectFolder.createDirectory();
    auto audioDir = getAudioAssetsDirectory (currentProjectFolder);
    auto midiDir = getMidiAssetsDirectory (currentProjectFolder);
    audioDir.createDirectory();
    midiDir.createDirectory();

    // Copy all files from cache into target Assets directory
    auto cacheDir = getTempCacheDirectory();
    if (cacheDir.exists())
    {
        for (const auto& f : cacheDir.findChildFiles (juce::File::findFiles, false))
        {
            f.copyFileTo (audioDir.getChildFile (f.getFileName()));
        }
    }

    return graph.saveProjectToFile (mainFile);
}

bool ProjectFileManager::loadProjectBundle (const juce::File& projectFolderOrFile, RelativisticNodeGraph& graph)
{
    juce::File mainFile;
    juce::File targetFolder;

    if (projectFolderOrFile.isDirectory())
    {
        targetFolder = projectFolderOrFile;
        mainFile = projectFolderOrFile.getChildFile ("patch.patch");
        if (!mainFile.existsAsFile()) mainFile = projectFolderOrFile.getChildFile ("project.xml");
        if (!mainFile.existsAsFile()) mainFile = projectFolderOrFile.getChildFile ("patch.xml");
        if (!mainFile.existsAsFile())
        {
            auto files = projectFolderOrFile.findChildFiles (juce::File::findFiles, false, "*.patch;*.json;*.xml");
            if (!files.isEmpty()) mainFile = files[0];
        }
    }
    else
    {
        mainFile = projectFolderOrFile;
        targetFolder = projectFolderOrFile.getParentDirectory();
    }

    if (!mainFile.existsAsFile()) return false;

    currentProjectFolder = targetFolder;
    return graph.loadProjectFromFile (mainFile);
}

} // namespace time_dilation
