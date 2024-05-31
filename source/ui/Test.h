//
// Created by Valentin Ackva on 06/05/2024.
//

#ifndef BINAURALPANNER_TEST_H
#define BINAURALPANNER_TEST_H

#include <JuceHeader.h>

#include <JuceHeader.h>

class OrbitalComponent : public juce::Component,
                         private juce::Timer
{
public:
    OrbitalComponent()
    {
        startTimerHz(60);  // Setting the timer for 60 frames per second
        setSize(500, 500);  // Set the size of the component
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);  // Background color
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();

        // Draw the planet at the center
        g.setColour(juce::Colours::grey);
        g.fillEllipse(center.x - 20, center.y - 20, 40, 40);  // Static size for the planet

        // Draw multiple orbits with varying inclinations and tracks
        drawOrbit(g, center, 100, 0.0f, 0.0f);   // Horizontal orbit
        drawOrbit(g, center, 120, 45.0f, 0.15f); // Tilted orbit at 45 degrees
        drawOrbit(g, center, 140, 90.0f, 0.3f);  // Vertical orbit
    }

    void timerCallback() override
    {
        orbitProgress += 0.005f;  // Increment the orbit progress
        if (orbitProgress >= 1.0f)
            orbitProgress = 0.0f;
        repaint();  // Redraw the component
    }

    void drawOrbit(juce::Graphics& g, juce::Point<float> center, float radius, float tiltDegrees, float progressOffset)
    {
        float orbitProgressLocal = fmod(orbitProgress + progressOffset, 1.0f);
        float angle = juce::MathConstants<float>::twoPi * orbitProgressLocal;
        float tiltRadians = juce::degreesToRadians(tiltDegrees);

        // Calculate the ellipse representing the orbit
        float majorAxis = radius * cos(tiltRadians);  // Adjust width based on tilt
        float minorAxis = radius * sin(tiltRadians);  // Adjust height based on tilt

        // Draw the orbit path
        g.setColour(juce::Colours::grey.withAlpha(0.5f));
        g.drawEllipse(center.x - majorAxis, center.y - minorAxis, 2 * majorAxis, 2 * minorAxis, 1.0f);

        // Position of the moving circle on this orbit
        juce::Point<float> orbitPoint(center.x + majorAxis * std::cos(angle),
                                      center.y + minorAxis * std::sin(angle));

        // Draw the moving circle
        g.setColour(juce::Colours::white);
        g.fillEllipse(orbitPoint.x - 5, orbitPoint.y - 5, 10, 10);  // Smaller circle size
    }

private:
    float orbitProgress = 0.0f;  // Normalized progress of each orbit
};

#endif //BINAURALPANNER_TEST_H
