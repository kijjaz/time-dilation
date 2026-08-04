#pragma once

#include "../dsp/AssetPoolManager.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace time_dilation
{

class AssetPoolDrawerComponent : public juce::Component,
                                 public juce::TableListBoxModel
{
public:
    AssetPoolDrawerComponent();
    ~AssetPoolDrawerComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void refreshPool();

    // TableListBoxModel overrides
    int getNumRows() override;
    void paintRowBackground (juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void cellClicked (int rowNumber, int columnId, const juce::MouseEvent& e) override;

    std::function<void()> onCloseRequested;

private:
    juce::TextButton btnClose       { "CLOSE POOL" };
    juce::TextButton btnImportAudio { "+ IMPORT AUDIO FILE" };
    juce::TextButton btnClearPool   { "CLEAR ALL" };
    juce::TableListBox tableList;

    std::vector<std::shared_ptr<PoolItem>> cachedItems;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AssetPoolDrawerComponent)
};

} // namespace time_dilation
