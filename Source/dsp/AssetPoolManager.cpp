#include "AssetPoolManager.h"
#include <juce_audio_formats/juce_audio_formats.h>

namespace time_dilation
{

std::string bitDepthToString (BitDepthFormat format)
{
    switch (format)
    {
        case BitDepthFormat::Int8:    return "8-bit Int";
        case BitDepthFormat::Int16:   return "16-bit Int";
        case BitDepthFormat::Int24:   return "24-bit Int";
        case BitDepthFormat::Float32: return "32-bit Float";
        case BitDepthFormat::Float64: return "64-bit Float";
    }
    return "24-bit Int";
}

BitDepthFormat stringToBitDepth (const std::string& str)
{
    juce::String s (str);
    s = s.toLowerCase();
    if (s.contains ("8")) return BitDepthFormat::Int8;
    if (s.contains ("16")) return BitDepthFormat::Int16;
    if (s.contains ("32") || s.contains ("float")) return BitDepthFormat::Float32;
    if (s.contains ("64") || s.contains ("double")) return BitDepthFormat::Float64;
    return BitDepthFormat::Int24;
}

AssetPoolManager& AssetPoolManager::getInstance()
{
    static AssetPoolManager instance;
    return instance;
}

int AssetPoolManager::addAudioItem (const std::string& name, const juce::AudioBuffer<float>& buffer, double sampleRate, BitDepthFormat format, int numChannels)
{
    const juce::ScopedLock lock (poolLock);
    auto item = std::make_shared<PoolItem>();
    item->id = nextId++;
    item->name = name.empty() ? ("Take " + std::to_string (item->id)) : name;
    item->type = PoolItemType::Audio;
    item->numChannels = numChannels;
    item->sampleRate = sampleRate;
    item->bitDepth = format;
    item->audioBuffer.makeCopyOf (buffer);
    item->durationSeconds = static_cast<double>(buffer.getNumSamples()) / (sampleRate > 0.0 ? sampleRate : 44100.0);
    item->durationBeats = (item->durationSeconds / 60.0) * 120.0;

    items.push_back (item);
    return item->id;
}

int AssetPoolManager::addImportedFile (const juce::File& file)
{
    if (!file.existsAsFile()) return -1;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

    if (!reader) return -1;

    const juce::ScopedLock lock (poolLock);
    auto item = std::make_shared<PoolItem>();
    item->id = nextId++;
    item->name = file.getFileNameWithoutExtension().toStdString();
    item->type = PoolItemType::Audio;
    item->numChannels = static_cast<int>(reader->numChannels);
    item->sampleRate = reader->sampleRate;
    item->filePath = file.getFullPathName();

    if (reader->bitsPerSample <= 8) item->bitDepth = BitDepthFormat::Int8;
    else if (reader->bitsPerSample <= 16) item->bitDepth = BitDepthFormat::Int16;
    else if (reader->bitsPerSample <= 24) item->bitDepth = BitDepthFormat::Int24;
    else if (reader->usesFloatingPointData) item->bitDepth = BitDepthFormat::Float32;
    else item->bitDepth = BitDepthFormat::Int24;

    int lengthSamples = static_cast<int>(reader->lengthInSamples);
    item->audioBuffer.setSize (item->numChannels, lengthSamples);
    reader->read (&item->audioBuffer, 0, lengthSamples, 0, true, true);

    item->durationSeconds = static_cast<double>(lengthSamples) / reader->sampleRate;
    item->durationBeats = (item->durationSeconds / 60.0) * 120.0;

    items.push_back (item);
    return item->id;
}

bool AssetPoolManager::removeItem (int id)
{
    const juce::ScopedLock lock (poolLock);
    auto it = std::remove_if (items.begin(), items.end(), [id] (const std::shared_ptr<PoolItem>& item) {
        return item->id == id;
    });
    if (it != items.end())
    {
        items.erase (it, items.end());
        return true;
    }
    return false;
}

std::shared_ptr<PoolItem> AssetPoolManager::getItem (int id) const
{
    const juce::ScopedLock lock (poolLock);
    for (const auto& item : items)
    {
        if (item->id == id) return item;
    }
    return nullptr;
}

std::vector<std::shared_ptr<PoolItem>> AssetPoolManager::getAllItems() const
{
    const juce::ScopedLock lock (poolLock);
    return items;
}

std::vector<std::shared_ptr<PoolItem>> AssetPoolManager::getItemsByType (PoolItemType type) const
{
    const juce::ScopedLock lock (poolLock);
    std::vector<std::shared_ptr<PoolItem>> result;
    for (const auto& item : items)
    {
        if (item->type == type) result.push_back (item);
    }
    return result;
}

void AssetPoolManager::clearPool()
{
    const juce::ScopedLock lock (poolLock);
    items.clear();
    nextId = 1;
}

juce::ValueTree AssetPoolManager::saveToValueTree() const
{
    const juce::ScopedLock lock (poolLock);
    juce::ValueTree poolTree ("AssetPool");
    for (const auto& item : items)
    {
        juce::ValueTree iTree ("PoolItem");
        iTree.setProperty ("id", item->id, nullptr);
        iTree.setProperty ("name", juce::String (item->name), nullptr);
        iTree.setProperty ("type", static_cast<int>(item->type), nullptr);
        iTree.setProperty ("numChannels", item->numChannels, nullptr);
        iTree.setProperty ("sampleRate", item->sampleRate, nullptr);
        iTree.setProperty ("bitDepth", juce::String (bitDepthToString (item->bitDepth)), nullptr);
        iTree.setProperty ("filePath", item->filePath, nullptr);
        iTree.setProperty ("durationSeconds", item->durationSeconds, nullptr);
        poolTree.addChild (iTree, -1, nullptr);
    }
    return poolTree;
}

void AssetPoolManager::loadFromValueTree (const juce::ValueTree& v)
{
    const juce::ScopedLock lock (poolLock);
    clearPool();
    if (!v.hasType ("AssetPool")) return;

    for (int i = 0; i < v.getNumChildren(); ++i)
    {
        auto c = v.getChild (i);
        if (c.hasType ("PoolItem"))
        {
            auto item = std::make_shared<PoolItem>();
            item->id = c.getProperty ("id", nextId++);
            item->name = c.getProperty ("name", "Asset").toString().toStdString();
            item->type = static_cast<PoolItemType>(static_cast<int>(c.getProperty ("type", 0)));
            item->numChannels = c.getProperty ("numChannels", 2);
            item->sampleRate = c.getProperty ("sampleRate", 44100.0);
            item->bitDepth = stringToBitDepth (c.getProperty ("bitDepth", "24-bit Int").toString().toStdString());
            item->filePath = c.getProperty ("filePath", "");
            item->durationSeconds = c.getProperty ("durationSeconds", 0.0);

            if (item->filePath.isNotEmpty())
            {
                juce::File f (item->filePath);
                if (f.existsAsFile())
                {
                    juce::AudioFormatManager formatManager;
                    formatManager.registerBasicFormats();
                    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (f));
                    if (reader)
                    {
                        item->audioBuffer.setSize (item->numChannels, static_cast<int>(reader->lengthInSamples));
                        reader->read (&item->audioBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
                    }
                }
            }

            items.push_back (item);
            if (item->id >= nextId) nextId = item->id + 1;
        }
    }
}

} // namespace time_dilation
