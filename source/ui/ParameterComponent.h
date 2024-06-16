//
// Created by Valentin Ackva on 16/06/2024.
//

#ifndef BINAURALPANNER_PARAMETERCOMPONENT_H
#define BINAURALPANNER_PARAMETERCOMPONENT_H

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class CustomGenericAudioProcessorEditor : public juce::GenericAudioProcessorEditor {
public:
    CustomGenericAudioProcessorEditor(juce::AudioProcessor& processor)
            : juce::GenericAudioProcessorEditor(processor) {
        setSize(400, 300);
    }

    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colour{0xff151517});
        g.fillAll();
    }
};

class ParameterComponent : public juce::Component {
public:
    explicit ParameterComponent(AudioPluginAudioProcessor& processor);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef;
    std::unique_ptr<CustomGenericAudioProcessorEditor> genericParameter;

    juce::LookAndFeel_V4 lf;

    std::unique_ptr<juce::Drawable> orbeLogo = juce::Drawable::createFromImageData(BinaryData::orbe_svg, BinaryData::orbe_svgSize);

};
#endif //BINAURALPANNER_PARAMETERCOMPONENT_H
