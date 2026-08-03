#include "GammaScriptEditorComponent.h"

namespace time_dilation
{

GammaScriptEditorComponent::GammaScriptEditorComponent (TimeDilationEngine& e)
    : engine (e)
{
    addAndMakeVisible (scriptEditor);
    scriptEditor.setMultiLine (true);
    scriptEditor.setReturnKeyStartsNewLine (true);
    scriptEditor.setFont (juce::FontOptions (13.0f, juce::Font::plain));
    scriptEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f141d));
    scriptEditor.setColour (juce::TextEditor::textColourId, juce::Colour (0xfff59e0b));
    scriptEditor.setText ("1.0 + sin(t * 2.0) * 0.5");

    addAndMakeVisible (applyButton);
    applyButton.onClick = [this] {
        if (currentTrackIndex >= 0 && currentTrackIndex < static_cast<int>(engine.getTracks().size()))
        {
            engine.getTracksMutable()[currentTrackIndex].gammaScriptCode = scriptEditor.getText();
            engine.getTracksMutable()[currentTrackIndex].isScriptEnabled = true;
        }
    };

    addAndMakeVisible (presetBox);
    loadPresets();
    presetBox.onChange = [this] {
        int id = presetBox.getSelectedId();
        if (id == 1) scriptEditor.setText ("1.0 + sin(t * 3.0) * 0.4");
        else if (id == 2) scriptEditor.setText ("1.0 - amp * 0.7");
        else if (id == 3) scriptEditor.setText ("tap1 * (step % 2 == 0 ? 1.5 : 0.8)");
        else if (id == 4) scriptEditor.setText ("exp(-tau * 0.1) * 2.0");
    };
}

void GammaScriptEditorComponent::loadPresets()
{
    presetBox.addItem ("Sine Wave Warp (1.0 + sin(t * 3.0) * 0.4)", 1);
    presetBox.addItem ("Audio Amplitude Sidechain Deceleration (1.0 - amp * 0.7)", 2);
    presetBox.addItem ("Polymetric Step Alternator", 3);
    presetBox.addItem ("Black Hole Decay (exp(-tau * 0.1) * 2.0)", 4);
}

void GammaScriptEditorComponent::setSelectedTrack (int trackIndex)
{
    currentTrackIndex = trackIndex;
    if (trackIndex >= 0 && trackIndex < static_cast<int>(engine.getTracks().size()))
    {
        scriptEditor.setText (engine.getTracks()[trackIndex].gammaScriptCode);
    }
}

void GammaScriptEditorComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff10141d));
    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("PROGRAMMABLE GAMMASCRIPT EDITOR", 10, 6, 300, 20, juce::Justification::left);
}

void GammaScriptEditorComponent::resized()
{
    presetBox.setBounds (getWidth() - 260, 6, 160, 24);
    applyButton.setBounds (getWidth() - 95, 6, 85, 24);
    scriptEditor.setBounds (10, 34, getWidth() - 20, getHeight() - 44);
}

} // namespace time_dilation
