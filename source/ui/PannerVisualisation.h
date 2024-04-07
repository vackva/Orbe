#ifndef BINAURALPANNER_PANNER_H
#define BINAURALPANNER_PANNER_H

#include <JuceHeader.h>

class PannerVisualisation : public juce::Component {
    void paint (juce::Graphics& g) override;

private:
    void drawCircles(juce::Graphics& g, juce::Rectangle<int> circleBounds);
    void drawLines(juce::Graphics& g, juce::Rectangle<int> circleBounds);

private:
    const int numberOfCircles = 4;
    const int reductionDivide = 10;
    const float initLineThickness = 1.5f;

    const juce::Colour boarderColour {0xff63748D};
    const juce::Colour circleFillColour {0xff63748D};
};


#endif //BINAURALPANNER_PANNER_H
