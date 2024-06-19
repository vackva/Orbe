//
// Created by Valentin Ackva on 17/06/2024.
//

#include "ParameterComponent.h"

ParameterComponent::ParameterComponent(AudioPluginAudioProcessor &processor) : processorRef(processor) {
    genericParameter = std::make_unique<CustomGenericAudioProcessorEditor>(processorRef);
    addAndMakeVisible(*genericParameter);
}

void ParameterComponent::paint(juce::Graphics &g) {
    g.setColour(juce::Colours::transparentBlack);
    g.fillAll();
    g.setColour(juce::Colour{0xff151517});
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);

    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop(static_cast<int>((float)getHeight() * 0.2f));
    orbeLogo->drawWithin(g, headerBounds.toFloat(), juce::RectanglePlacement::doNotResize, 1.f);
}

void ParameterComponent::resized() {
    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop(static_cast<int>((float)getHeight() * 0.2f));
    bounds.removeFromBottom(10);
    auto paramBounds = bounds;

    genericParameter->setBounds(paramBounds);
}
