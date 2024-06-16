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
        // Set the size if needed
        setSize(400, 300);

    }

    void paint(juce::Graphics& g) override {
        // Set the background color
        g.setColour(juce::Colour{0xff151517});
        g.fillAll();
    }
};
class ParameterComponent : public juce::Component {
public:
    ParameterComponent(AudioPluginAudioProcessor& processor) : processorRef(processor) {
        // Use the custom generic editor
        genericParameter = std::make_unique<CustomGenericAudioProcessorEditor>(processorRef);
        addAndMakeVisible(*genericParameter);
    }

    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colours::transparentBlack);
        g.fillAll();
        g.setColour(juce::Colour{0xff151517});
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);

        auto bounds = getLocalBounds();
        auto headerBounds = bounds.removeFromTop(static_cast<int>((float)getHeight() * 0.2f));
        orbeLogo->drawWithin(g, headerBounds.toFloat(), juce::RectanglePlacement::doNotResize, 1.f);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        auto headerBounds = bounds.removeFromTop(static_cast<int>((float)getHeight() * 0.2f));
        bounds.removeFromBottom(10);
        auto paramBounds = bounds;

        genericParameter->setBounds(paramBounds);
    }

private:
    AudioPluginAudioProcessor& processorRef;
    std::unique_ptr<CustomGenericAudioProcessorEditor> genericParameter;

    juce::LookAndFeel_V4 lf;

    std::unique_ptr<juce::Drawable> orbeLogo = juce::Drawable::createFromImageData(BinaryData::orbe_svg, BinaryData::orbe_svgSize);

};
#endif //BINAURALPANNER_PARAMETERCOMPONENT_H
