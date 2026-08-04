#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace time_dilation
{

enum class PoolItemType
{
    Audio,
    Midi,
    Control,
    TimeDilation
};

enum class BitDepthFormat
{
    Int8,
    Int16,
    Int24,
    Float32,
    Float64
};

std::string bitDepthToString (BitDepthFormat format);
BitDepthFormat stringToBitDepth (const std::string& str);

struct PoolItem
{
    int id = 0;
    std::string name = "Asset";
    PoolItemType type = PoolItemType::Audio;
    int numChannels = 2;
    double sampleRate = 44100.0;
    BitDepthFormat bitDepth = BitDepthFormat::Int24;

    juce::AudioBuffer<float> audioBuffer;
    juce::String filePath;
    double durationSeconds = 0.0;
    double durationBeats = 4.0;
};

class AssetPoolManager
{
public:
    static AssetPoolManager& getInstance();

    int addAudioItem (const std::string& name, const juce::AudioBuffer<float>& buffer, double sampleRate, BitDepthFormat format, int numChannels);
    int addImportedFile (const juce::File& file);
    
    bool removeItem (int id);
    std::shared_ptr<PoolItem> getItem (int id) const;
    std::vector<std::shared_ptr<PoolItem>> getAllItems() const;
    std::vector<std::shared_ptr<PoolItem>> getItemsByType (PoolItemType type) const;

    void clearPool();

    juce::ValueTree saveToValueTree() const;
    void loadFromValueTree (const juce::ValueTree& v);

private:
    AssetPoolManager() = default;
    mutable juce::CriticalSection poolLock;
    std::vector<std::shared_ptr<PoolItem>> items;
    int nextId = 1;
};

} // namespace time_dilation
