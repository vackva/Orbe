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
    float normalizedX = processorRef.getValueTreeState().getParameter("param_x")->getValue();
    float normalizedY = processorRef.getValueTreeState().getParameter("param_y")->getValue();
    float normalizedZ = processorRef.getValueTreeState().getParameter("param_z")->getValue();

    float x = PluginParameters::xRange.convertFrom0to1(normalizedX);
    float y = PluginParameters::yRange.convertFrom0to1(normalizedY);
    float z = PluginParameters::zRange.convertFrom0to1(normalizedZ);

    pannerVisualisation.setVisualPosition(x, y, z);
}

void AudioPluginAudioProcessorEditor::pannerChanged(float x, float y) {
    auto& processorParams = processorRef.getValueTreeState();

    auto paramX = processorParams.getParameter(PluginParameters::X_ID.getParamID());
    auto paramY = processorParams.getParameter(PluginParameters::Y_ID.getParamID());

    paramX->setValueNotifyingHost(paramX->convertTo0to1(x));
    paramY->setValueNotifyingHost(paramY->convertTo0to1(y));
}

