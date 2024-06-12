#include "PannerVisualisation.h"

PannerVisualisation::PannerVisualisation() : isInitialized(false) {
    smallCircleRadius = 10.0f;
}

void PannerVisualisation::paint(juce::Graphics &g) {
    // Initializing the smallCirclePosition here, because the bounds are not available in the constructor
    if (!isInitialized) {
        smallCirclePosition = getLocalBounds().getCentre().toFloat();
        isInitialized = true;
        setVisualPosition(0.0f, 0.0f, 0.0f);
    }
    const auto circleBounds = getLocalBounds().reduced(getWidth() / reductionDivide);

    drawCircles(g, circleBounds);
    drawLines(g, circleBounds);
    drawSmallCircle(g);
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

void PannerVisualisation::drawSmallCircle(juce::Graphics &g) {
    // const float smallCircleRadius = 10.0f; // Radius of the small circle
    g.setColour(juce::Colours::lightgrey); // Small circle color
    g.fillEllipse(smallCirclePosition.x - smallCircleRadius, smallCirclePosition.y - smallCircleRadius, smallCircleRadius * 2, smallCircleRadius * 2);
}

void PannerVisualisation::mouseDown(const juce::MouseEvent &event) {
    if (isInsideSmallCircle(event.position)) {
        dragging = true;
        mouseIsActive = true;
    }
}

void PannerVisualisation::mouseDrag(const juce::MouseEvent &event) {
    auto bounds = getLocalBounds().reduced(getWidth() / reductionDivide).toFloat();
    auto center = bounds.getCentre();
    auto radius = static_cast<float>(bounds.getWidth() / 2.0);
    juce::Point<float> newPos = event.position.toFloat();
    float distanceX = std::abs(newPos.x - center.x);
    float distanceY = std::abs(newPos.y - center.y);

    if (distanceX > radius) {
        newPos.x = (newPos.x > center.x) ? center.x + radius : center.x - radius;
    }
    if (distanceY > radius) {
        newPos.y = (newPos.y > center.y) ? center.y + radius : center.y - radius;
    }

    smallCirclePosition = newPos;
    float newX = - (newPos.y - center.y) / radius * 10.0f;
    float newY = - (newPos.x - center.x) / radius * 10.0f;

    listeners.call([newX, newY](Listener& l) { l.pannerChanged(newX, newY); });

    repaint();
}

void PannerVisualisation::mouseUp(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    dragging = false;
    mouseIsActive = false;
}

bool PannerVisualisation::isInsideSmallCircle(const juce::Point<float> &point) {
    return smallCirclePosition.getDistanceFrom(point) <= 50.0;
}

void PannerVisualisation::setVisualPosition(float x, float y, float z) {
    if (approximatelyEqual(x, lastX) && approximatelyEqual(y, lastY) && approximatelyEqual(z, lastZ)) {
        return;
    }
    lastX = x;
    lastY = y;
    lastZ = z;

    auto bounds = getLocalBounds().reduced(getWidth() / reductionDivide);
    auto center = bounds.getCentre().toFloat();
    float outerRadius = static_cast<float>(bounds.getWidth() / 2.0);

    float zScale = ((z + 10) / 20.0f) * 1.5f + 0.5f;
    smallCircleRadius = 10.0f * zScale;

    float newX = center.x - (y / 10.0f) * outerRadius;
    float newY = center.y - (x / 10.0f) * outerRadius;

    smallCirclePosition.setXY(newX, newY);

    repaint();
}


void PannerVisualisation::addListener(PannerVisualisation::Listener *listenerToAdd) {
    listeners.add(listenerToAdd);
}

void PannerVisualisation::removeListener(PannerVisualisation::Listener *listenerToRemove) {
    listeners.remove(listenerToRemove);
}
