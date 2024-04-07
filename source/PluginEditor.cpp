#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    addAndMakeVisible(backgroundComponent);
    addAndMakeVisible(pannerVisualisation);
    setSize (400, 300);

    genericParameter = std::make_unique<juce::GenericAudioProcessorEditor>(p);
    addAndMakeVisible(*genericParameter);

    setEditorDimensions();
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

void AudioPluginAudioProcessorEditor::resized()
{
    const auto backgroundBounds = getLocalBounds();
    backgroundComponent.setBounds(backgroundBounds);
    const auto pannerVisualisationBounds = getLocalBounds().removeFromTop(getWidth());
    pannerVisualisation.setBounds(pannerVisualisationBounds);
    const auto parameterBounds = getLocalBounds().removeFromBottom(getHeight() - pannerVisualisationBounds.getHeight());
    if (genericParameter != nullptr) {
        genericParameter->setBounds(parameterBounds);
    }
}
