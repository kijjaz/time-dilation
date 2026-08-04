#include "AssetPoolDrawerComponent.h"
#include "FontManager.h"

namespace time_dilation
{

AssetPoolDrawerComponent::AssetPoolDrawerComponent()
{
    addAndMakeVisible (btnClose);
    btnClose.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1e293b));
    btnClose.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfff8fafc));
    btnClose.onClick = [this] { if (onCloseRequested) onCloseRequested(); };

    addAndMakeVisible (btnImportAudio);
    btnImportAudio.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f766e));
    btnImportAudio.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff38bdf8));
    btnImportAudio.onClick = [this] {
        auto chooser = std::make_shared<juce::FileChooser> ("Import Audio File into Workstation Pool...", juce::File(), "*.wav;*.aif;*.aiff;*.flac;*.mp3");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    AssetPoolManager::getInstance().addImportedFile (file);
                    refreshPool();
                }
            });
    };

    addAndMakeVisible (btnClearPool);
    btnClearPool.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff881337));
    btnClearPool.setColour (juce::TextButton::textColourOffId, juce::Colour (0xfffca5a5));
    btnClearPool.onClick = [this] {
        AssetPoolManager::getInstance().clearPool();
        refreshPool();
    };

    addAndMakeVisible (tableList);
    tableList.setModel (this);
    tableList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xff070a12));

    tableList.getHeader().addColumn ("ID", 1, 50);
    tableList.getHeader().addColumn ("ASSET NAME", 2, 220);
    tableList.getHeader().addColumn ("TYPE", 3, 90);
    tableList.getHeader().addColumn ("CHANNELS", 4, 90);
    tableList.getHeader().addColumn ("BIT DEPTH", 5, 110);
    tableList.getHeader().addColumn ("DURATION", 6, 100);

    refreshPool();
}

void AssetPoolDrawerComponent::refreshPool()
{
    cachedItems = AssetPoolManager::getInstance().getAllItems();
    tableList.updateContent();
    repaint();
}

void AssetPoolDrawerComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0f1d));

    g.setColour (juce::Colour (0xff1e293b));
    g.drawRect (getLocalBounds(), 2.0f);

    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::Font ("Arial", 16.0f, juce::Font::bold));
    g.drawText ("GLOBAL WORKSPACE MEDIA & RESOURCE POOL", 16, 12, 400, 24, juce::Justification::left);
}

void AssetPoolDrawerComponent::resized()
{
    btnClose.setBounds (getWidth() - 120, 10, 105, 26);
    btnImportAudio.setBounds (16, 44, 180, 26);
    btnClearPool.setBounds (204, 44, 110, 26);

    tableList.setBounds (12, 78, getWidth() - 24, getHeight() - 90);
}

int AssetPoolDrawerComponent::getNumRows()
{
    return static_cast<int>(cachedItems.size());
}

void AssetPoolDrawerComponent::paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    juce::Colour bg = (rowNumber % 2 == 0) ? juce::Colour (0xff0d1322) : juce::Colour (0xff0a0f1d);
    if (rowIsSelected) bg = juce::Colour (0xff1e293b);
    g.fillAll (bg);
}

void AssetPoolDrawerComponent::paintCell (juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(cachedItems.size())) return;
    const auto& item = cachedItems[static_cast<size_t>(rowNumber)];

    g.setColour (juce::Colour (0xfff8fafc));
    g.setFont (juce::Font ("Arial", 13.0f, juce::Font::plain));

    juce::String text = "";
    if (columnId == 1) text = "#" + juce::String (item->id);
    else if (columnId == 2) text = item->name;
    else if (columnId == 3) text = (item->type == PoolItemType::Audio) ? "Audio" : "Data";
    else if (columnId == 4) text = (item->numChannels == 1) ? "1 (Mono)" : ((item->numChannels == 2) ? "2 (Stereo)" : (juce::String (item->numChannels) + " Ch"));
    else if (columnId == 5) text = bitDepthToString (item->bitDepth);
    else if (columnId == 6) text = juce::String (item->durationSeconds, 2) + "s";

    g.drawText (text, 6, 0, width - 12, height, juce::Justification::centredLeft, true);
}

void AssetPoolDrawerComponent::cellClicked (int rowNumber, int /*columnId*/, const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown() && rowNumber >= 0 && rowNumber < static_cast<int>(cachedItems.size()))
    {
        auto item = cachedItems[static_cast<size_t>(rowNumber)];
        juce::PopupMenu m;
        m.addItem (1, "Delete Pool Asset");
        m.showMenuAsync (juce::PopupMenu::Options(), [this, item] (int res) {
            if (res == 1)
            {
                AssetPoolManager::getInstance().removeItem (item->id);
                refreshPool();
            }
        });
    }
}

} // namespace time_dilation
