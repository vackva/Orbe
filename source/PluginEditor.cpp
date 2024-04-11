#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    addAndMakeVisible(backgroundComponent);
    addAndMakeVisible(pannerVisualisation);

    genericParameter = std::make_unique<juce::GenericAudioProcessorEditor>(p);
    addAndMakeVisible(*genericParameter);

    setEditorDimensions();

    startTimerHz(30);

    pannerVisualisation.addListener(this);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    stopTimer();
    pannerVisualisation.removeListener(this);
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

void AudioPluginAudioProcessorEditor::setEditorDimensions() {
    double ratio = 1.0/1.75;

    setResizeLimits(350,
                    static_cast<int>(350.0 / ratio),
                    800,
                    static_cast<int>(800.0 / ratio));

    getConstrainer()->setFixedAspectRatio(ratio);

    setSize(400,static_cast<int>(400.0 / ratio));
}

void AudioPluginAudioProcessorEditor::timerCallback() {
    float paramAzim = processorRef.getAtomicParameterValue(PluginParameters::AZIM_ID.getParamID());
    float paramElev = processorRef.getAtomicParameterValue(PluginParameters::ELEV_ID.getParamID());
    pannerVisualisation.setAzimuthAndElevation(paramAzim, paramElev);
}

void AudioPluginAudioProcessorEditor::pannerChanged(float azimuth, float elevation) {
    auto& processorParams = processorRef.getValueTreeState();

    auto paramAzim = processorParams.getParameter(PluginParameters::AZIM_ID.getParamID());
    auto paramElev = processorParams.getParameter(PluginParameters::ELEV_ID.getParamID());

    paramAzim->setValueNotifyingHost(paramAzim->convertTo0to1(azimuth));
    paramElev->setValueNotifyingHost(paramElev->convertTo0to1(elevation));
}
