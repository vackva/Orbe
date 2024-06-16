//
// Created by Valentin Ackva on 16/06/2024.
//

#ifndef BINAURALPANNER_PANNERCOMPONENT_H
#define BINAURALPANNER_PANNERCOMPONENT_H

#include <JuceHeader.h>
#include "ui/PannerVisualisation.h"

class PannerComponent : public juce::Component, juce::Timer, PannerVisualisation::Listener {
public:
    explicit PannerComponent(AudioPluginAudioProcessor& processor) : processorRef(processor), viewButton("viewButton", juce::DrawableButton::ButtonStyle::ImageFitted) {
        addAndMakeVisible(pannerVisualisation);
        pannerVisualisation.addListener(this);
        startTimerHz(30);

        viewButton.setClickingTogglesState(true);
        viewButton.setImages(button2d.get(),
        button2d.get(),
        button3d.get(),
        button2d.get(),
        button3d.get(),
        button3d.get(),
        button3d.get(),
        button3d.get());
        addAndMakeVisible(viewButton);
    }

    ~PannerComponent() override {
        pannerVisualisation.removeListener(this);
        stopTimer();
    }

private:

    void paint (juce::Graphics& g) override {
        g.setColour(juce::Colour {0xff151517});
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
    }

    void resized() override {
        pannerVisualisation.setBounds(getLocalBounds().reduced(10));
        juce::Rectangle<int> button (getWidth() - 68, getHeight() - 18, 68, 18);
        viewButton.setBounds(button);
    }

    void pannerChanged(float x, float y) override {
        auto& processorParams = processorRef.getValueTreeState();

        auto paramX = processorParams.getParameter(PluginParameters::X_ID.getParamID());
        auto paramY = processorParams.getParameter(PluginParameters::Y_ID.getParamID());

        paramX->setValueNotifyingHost(paramX->convertTo0to1(x));
        paramY->setValueNotifyingHost(paramY->convertTo0to1(y));
    }

    void timerCallback() override {
        float normalizedX = processorRef.getValueTreeState().getParameter("param_x")->getValue();
        float normalizedY = processorRef.getValueTreeState().getParameter("param_y")->getValue();
        float normalizedZ = processorRef.getValueTreeState().getParameter("param_z")->getValue();

        float x = PluginParameters::xRange.convertFrom0to1(normalizedX);
        float y = PluginParameters::yRange.convertFrom0to1(normalizedY);
        float z = PluginParameters::zRange.convertFrom0to1(normalizedZ);

        pannerVisualisation.setVisualPosition(x, y, z);
    }

private:
    AudioPluginAudioProcessor& processorRef;
    PannerVisualisation pannerVisualisation;

    std::unique_ptr<juce::Drawable> button2d = juce::Drawable::createFromImageData(BinaryData::button_2d_svg, BinaryData::button_2d_svgSize);
    std::unique_ptr<juce::Drawable> button3d = juce::Drawable::createFromImageData(BinaryData::button_3d_svg, BinaryData::button_3d_svgSize);
    juce::DrawableButton viewButton;
};

#endif //BINAURALPANNER_PANNERCOMPONENT_H
