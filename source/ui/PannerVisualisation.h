#ifndef BINAURALPANNER_PANNER_H
#define BINAURALPANNER_PANNER_H

#include <JuceHeader.h>

class PannerVisualisation : public juce::Component {
public:
    PannerVisualisation();

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void pannerChanged(float azimuth, float elevation) { ignoreUnused(azimuth, elevation); }
    };

    void addListener(Listener* listenerToAdd);
    void removeListener(Listener* listenerToRemove);

    void paint (juce::Graphics& g) override;
    void setAzimuthAndElevation(float azimuth, float elevation);

private:
    void drawCircles(juce::Graphics& g, juce::Rectangle<int> circleBounds);
    void drawLines(juce::Graphics& g, juce::Rectangle<int> circleBounds);
    void drawSmallCircle(juce::Graphics& g);

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    bool isInsideSmallCircle(const juce::Point<float>& point);
    float calculateAzimuth() const;

    float calculateElevation() const;

private:
    const int numberOfCircles = 4;
    const int reductionDivide = 10;
    const float initLineThickness = 1.5f;

    float lastAzimuth = 0.0f;
    float lastElevation = 0.0f;
    bool mouseIsActive = false;


    juce::Point<float> smallCirclePosition;
    bool dragging = false;

    const juce::Colour boarderColour {0xff63748D};
    const juce::Colour circleFillColour {0xff63748D};

    ListenerList<Listener> listeners;
};


#endif //BINAURALPANNER_PANNER_H
