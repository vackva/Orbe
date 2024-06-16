//
// Created by Valentin Ackva on 16/06/2024.
//

#ifndef BINAURALPANNER_PANNERCOMPONENT_H
#define BINAURALPANNER_PANNERCOMPONENT_H

#include <JuceHeader.h>
#include "PannerVisualisation.h"
#include "opengl/Panner3dOpenGL.h"
#include "../PluginProcessor.h"

class PannerComponent : public juce::Component, juce::Timer, PannerVisualisation::Listener {
public:
    explicit PannerComponent(AudioPluginAudioProcessor& processor);
    ~PannerComponent() override;

private:
    void paint (juce::Graphics& g) override;
    void resized() override;

    void pannerChanged(float x, float y) override;
    void timerCallback() override;

private:
    AudioPluginAudioProcessor& processorRef;
    PannerVisualisation pannerVisualisation;
    MainContentComponent openGLVisualisation;

    std::unique_ptr<juce::Drawable> button2d = juce::Drawable::createFromImageData(BinaryData::button_2d_svg, BinaryData::button_2d_svgSize);
    std::unique_ptr<juce::Drawable> button3d = juce::Drawable::createFromImageData(BinaryData::button_3d_svg, BinaryData::button_3d_svgSize);
    juce::DrawableButton viewButton;

    bool show2D = true;
};

#endif //BINAURALPANNER_PANNERCOMPONENT_H
