//
// Created by Valentin Ackva on 17/06/2024.
//

#include "PannerComponent.h"

PannerComponent::PannerComponent(AudioPluginAudioProcessor &processor) : processorRef(processor), viewButton("viewButton", juce::DrawableButton::ButtonStyle::ImageFitted) {
    addAndMakeVisible(pannerVisualisation);
    addAndMakeVisible(openGLVisualisation);
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

    viewButton.onClick = [this]() {
        if (show2D) {
            show2D = false;
            pannerVisualisation.setVisible(false);
            openGLVisualisation.setVisible(true);
        } else {
            show2D = true;
            pannerVisualisation.setVisible(true);
            openGLVisualisation.setVisible(false);
        }
    };

    openGLVisualisation.setVisible(false);

    addAndMakeVisible(viewButton);
}

PannerComponent::~PannerComponent() {
    pannerVisualisation.removeListener(this);
    stopTimer();
}

void PannerComponent::paint(Graphics &g) {
    g.setColour(juce::Colour {0xff151517});
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
}

void PannerComponent::resized() {
    pannerVisualisation.setBounds(getLocalBounds().reduced(10));
    openGLVisualisation.setBounds(getLocalBounds().reduced(20));

    juce::Rectangle<int> button (getWidth() - 68, getHeight() - 18, 68, 18);
    viewButton.setBounds(button);
}

void PannerComponent::pannerChanged(float x, float y) {
    auto& processorParams = processorRef.getValueTreeState();

    auto paramX = processorParams.getParameter(PluginParameters::X_ID.getParamID());
    auto paramY = processorParams.getParameter(PluginParameters::Y_ID.getParamID());

    paramX->setValueNotifyingHost(paramX->convertTo0to1(x));
    paramY->setValueNotifyingHost(paramY->convertTo0to1(y));
}

void PannerComponent::timerCallback() {
    float normalizedX = processorRef.getValueTreeState().getParameter("param_x")->getValue();
    float normalizedY = processorRef.getValueTreeState().getParameter("param_y")->getValue();
    float normalizedZ = processorRef.getValueTreeState().getParameter("param_z")->getValue();

    float x = PluginParameters::xRange.convertFrom0to1(normalizedX);
    float y = PluginParameters::yRange.convertFrom0to1(normalizedY);
    float z = PluginParameters::zRange.convertFrom0to1(normalizedZ);

    if (show2D) {
        pannerVisualisation.setVisualPosition(x, y, z);
    } else {
        openGLVisualisation.setVisualPosition(x, y, z);
    }
}
