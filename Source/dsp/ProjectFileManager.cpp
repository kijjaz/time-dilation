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
    juce::File xmlFile;
    if (targetProjectFileOrFolder.isDirectory())
    {
        currentProjectFolder = targetProjectFileOrFolder;
        xmlFile = targetProjectFileOrFolder.getChildFile ("project.xml");
    }
    else
    {
        currentProjectFolder = targetProjectFileOrFolder.getParentDirectory();
        xmlFile = targetProjectFileOrFolder;
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

    // Save graph ValueTree XML
    auto state = graph.saveToValueTree();
    auto xml = state.createXml();
    if (xml != nullptr)
    {
        return xml->writeTo (xmlFile);
    }

    return false;
}

bool ProjectFileManager::loadProjectBundle (const juce::File& projectFolderOrFile, RelativisticNodeGraph& graph)
{
    juce::File xmlFile;
    juce::File targetFolder;

    if (projectFolderOrFile.isDirectory())
    {
        targetFolder = projectFolderOrFile;
        xmlFile = projectFolderOrFile.getChildFile ("project.xml");
        if (!xmlFile.existsAsFile()) xmlFile = projectFolderOrFile.getChildFile ("patch.xml");
        if (!xmlFile.existsAsFile())
        {
            auto xmlFiles = projectFolderOrFile.findChildFiles (juce::File::findFiles, false, "*.xml");
            if (!xmlFiles.isEmpty()) xmlFile = xmlFiles[0];
        }
    }
    else
    {
        xmlFile = projectFolderOrFile;
        targetFolder = projectFolderOrFile.getParentDirectory();
    }

    if (!xmlFile.existsAsFile()) return false;

    currentProjectFolder = targetFolder;

    auto xml = juce::XmlDocument::parse (xmlFile);
    if (xml != nullptr)
    {
        auto state = juce::ValueTree::fromXml (*xml);
        if (state.isValid())
        {
            graph.loadFromValueTree (state);
            return true;
        }
    }

    return false;
}

} // namespace time_dilation
