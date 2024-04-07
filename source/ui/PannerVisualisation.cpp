#include "PannerVisualisation.h"

void PannerVisualisation::paint(juce::Graphics &g) {
    const auto circleBounds = getLocalBounds().reduced(getWidth() / reductionDivide);

    drawCircles(g, circleBounds);
    drawLines(g, circleBounds);
}

void PannerVisualisation::drawCircles(juce::Graphics &g, juce::Rectangle<int> circleBounds) {
    const int reduction = getWidth() / reductionDivide;

    for (int i = 0; i < numberOfCircles; ++i) {
        g.setColour(boarderColour);
        juce::Rectangle<float> bounds = circleBounds.reduced(reduction * i).toFloat();
        float lineThickness = initLineThickness - ((float) i * 0.1f);
        g.drawEllipse(bounds, lineThickness);

        g.setColour(circleFillColour);
        g.setOpacity(0.15f);
        g.fillEllipse(bounds);
    }
}

void PannerVisualisation::drawLines(juce::Graphics &g, juce::Rectangle<int> circleBounds) {
    const auto centre = circleBounds.getCentre().toFloat();
    const float radius = (float) circleBounds.getWidth() / 2.0f;

    const float cos45 = std::cos(juce::MathConstants<float>::pi / 4.0f);
    const float diagRadius = radius * cos45;

    juce::Point<float> diagStart1(centre.x - diagRadius, centre.y - diagRadius);
    juce::Point<float> diagEnd1(centre.x + diagRadius, centre.y + diagRadius);
    juce::Point<float> diagStart2(centre.x - diagRadius, centre.y + diagRadius);
    juce::Point<float> diagEnd2(centre.x + diagRadius, centre.y - diagRadius);

    g.setColour(boarderColour);
    g.setOpacity(1.0f);

    // Horizontal and vertical lines
    g.drawLine(centre.x - radius, centre.y, centre.x + radius, centre.y, initLineThickness);
    g.drawLine(centre.x, centre.y - radius, centre.x, centre.y + radius, initLineThickness);

    // Drawing diagonal lines
    g.drawLine(diagStart1.x, diagStart1.y, diagEnd1.x, diagEnd1.y, initLineThickness);
    g.drawLine(diagStart2.x, diagStart2.y, diagEnd2.x, diagEnd2.y, initLineThickness);
}
