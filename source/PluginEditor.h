#pragma once

#include "PluginProcessor.h"
#include "PluginParameters.h"
#include "ui/BackgroundComponent.h"
#include "ui/PannerVisualisation.h"

#include "ui/PannerComponent.h"
#include "ui/ParameterComponent.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, juce::Timer, PannerVisualisation::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void setEditorDimensions();
    void timerCallback() override;
    void pannerChanged(float x, float y);


private:
    AudioPluginAudioProcessor& processorRef;

    BackgroundComponent backgroundComponent;

    ParameterComponent parameterComponent;
    PannerComponent pannerComponent;

    PannerVisualisation pannerVisualisation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
