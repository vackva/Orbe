#ifndef BINAURALPANNER_PANNER_H
#define BINAURALPANNER_PANNER_H

#include <JuceHeader.h>

class PannerVisualisation : public juce::Component {
public:
    PannerVisualisation();

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void pannerChanged(float x, float y) { ignoreUnused(x, y); }
    };

    void addListener(Listener* listenerToAdd);
    void removeListener(Listener* listenerToRemove);

    void paint (juce::Graphics& g) override;
    void setVisualPosition(float x, float y, float z);

private:
    void drawCircles(juce::Graphics& g, juce::Rectangle<int> circleBounds);
    void drawLines(juce::Graphics& g, juce::Rectangle<int> circleBounds);
    void drawSmallCircle(juce::Graphics& g);

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    bool isInsideSmallCircle(const juce::Point<float>& point);

    float smallCircleRadius;

private:
    const int numberOfCircles = 4;
    const int reductionDivide = 10;
    const float initLineThickness = 1.5f;

    bool mouseIsActive = false;

    float lastX = 0.0f;
    float lastY = 0.0f;
    float lastZ = 0.0f;

    juce::Point<float> smallCirclePosition;
    bool dragging = false;
    bool isInitialized;

    const juce::Colour boarderColour {0xff63748D};
    const juce::Colour circleFillColour {0xff63748D};

    ListenerList<Listener> listeners;
};


#endif //BINAURALPANNER_PANNER_H
