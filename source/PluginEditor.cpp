#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
        : AudioProcessorEditor (&p), processorRef (p), parameterComponent(p), pannerComponent(p)
{
    juce::ignoreUnused (processorRef);

    addAndMakeVisible(parameterComponent);
    addAndMakeVisible(pannerComponent);

    setEditorDimensions();
}



AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{

}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour {0xff000000});
    g.fillAll();
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(getWidth() / 60);

    auto pannerBounds = bounds.removeFromRight(bounds.getHeight());
    auto gap = bounds.removeFromRight(getWidth() / 60);
    auto paramBounds = bounds;

    parameterComponent.setBounds(paramBounds);
    pannerComponent.setBounds(pannerBounds);
}

void AudioPluginAudioProcessorEditor::setEditorDimensions() {
    double ratio = 1000.0/600.0;

    setResizeLimits(600,
                    static_cast<int>(600.0 / ratio),
                    1200,
                    static_cast<int>(1200.0 / ratio));

    getConstrainer()->setFixedAspectRatio(ratio);

    setSize(1200,static_cast<int>(1200.0 / ratio));
}
