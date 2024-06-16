#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
        : AudioProcessorEditor (&p), processorRef (p), parameterComponent(p), pannerComponent(p)
{
    juce::ignoreUnused (processorRef);
//    addAndMakeVisible(backgroundComponent);
//    addAndMakeVisible(pannerVisualisation);
//
//    genericParameter = std::make_unique<juce::GenericAudioProcessorEditor>(p);
//    addAndMakeVisible(*genericParameter);


    addAndMakeVisible(parameterComponent);
    addAndMakeVisible(pannerComponent);

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
    g.setColour(juce::Colour {0xff000000});
    g.fillAll();
}

void AudioPluginAudioProcessorEditor::resized()
{
//    const auto backgroundBounds = getLocalBounds();
//    backgroundComponent.setBounds(backgroundBounds);
//    const auto pannerVisualisationBounds = getLocalBounds().removeFromTop(getWidth());
//    pannerVisualisation.setBounds(pannerVisualisationBounds);
//    const auto parameterBounds = getLocalBounds().removeFromBottom(getHeight() - pannerVisualisationBounds.getHeight());
//    if (genericParameter != nullptr) {
//        genericParameter->setBounds(parameterBounds);
//    }

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

void AudioPluginAudioProcessorEditor::timerCallback() {
//    float normalizedX = processorRef.getValueTreeState().getParameter("param_x")->getValue();
//    float normalizedY = processorRef.getValueTreeState().getParameter("param_y")->getValue();
//    float normalizedZ = processorRef.getValueTreeState().getParameter("param_z")->getValue();
//
//    float x = PluginParameters::xRange.convertFrom0to1(normalizedX);
//    float y = PluginParameters::yRange.convertFrom0to1(normalizedY);
//    float z = PluginParameters::zRange.convertFrom0to1(normalizedZ);
//
//    pannerVisualisation.setVisualPosition(x, y, z);
}

void AudioPluginAudioProcessorEditor::pannerChanged(float x, float y) {
    auto& processorParams = processorRef.getValueTreeState();

    auto paramX = processorParams.getParameter(PluginParameters::X_ID.getParamID());
    auto paramY = processorParams.getParameter(PluginParameters::Y_ID.getParamID());

    paramX->setValueNotifyingHost(paramX->convertTo0to1(x));
    paramY->setValueNotifyingHost(paramY->convertTo0to1(y));
}

