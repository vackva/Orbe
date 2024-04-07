#pragma once

#include "PluginProcessor.h"
#include "ui/BackgroundComponent.h"
#include "ui/PannerVisualisation.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void setEditorDimensions() {
        double ratio = 1.0/1.75;

        setResizeLimits(350,
                        static_cast<int>(350.0 / ratio),
                        800,
                        static_cast<int>(800.0 / ratio));

        getConstrainer()->setFixedAspectRatio(ratio);

        setSize(400,static_cast<int>(400.0 / ratio));
    }

private:
    AudioPluginAudioProcessor& processorRef;

    BackgroundComponent backgroundComponent;
    PannerVisualisation pannerVisualisation;
    std::unique_ptr<juce::GenericAudioProcessorEditor> genericParameter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
