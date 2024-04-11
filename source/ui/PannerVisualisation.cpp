#include "PannerVisualisation.h"

PannerVisualisation::PannerVisualisation() {

}

void PannerVisualisation::paint(juce::Graphics &g) {
    const auto circleBounds = getLocalBounds().reduced(getWidth() / reductionDivide);

    if (!circleBounds.contains(smallCirclePosition.toInt())) {
        const auto bounds = getLocalBounds();
        smallCirclePosition = bounds.getCentre().toFloat();
    }

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
    const float smallCircleRadius = 10.0f; // Radius of the small circle
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
    if (dragging) {
        auto bounds = getLocalBounds().reduced(getWidth() / reductionDivide).toFloat();
        juce::Point<float> newPos = event.position.toFloat();
        auto center = bounds.getCentre();
        auto vectorFromCenter = newPos - center;
        auto distanceToCenter = vectorFromCenter.getDistanceFrom({0, 0});

        auto radius = static_cast<float>(bounds.getWidth() / 2.0);
        if (distanceToCenter > radius) {
            float normX = vectorFromCenter.x / distanceToCenter;
            float normY = vectorFromCenter.y / distanceToCenter;
            newPos.x = center.x + normX * radius;
            newPos.y = center.y + normY * radius;
        }
        smallCirclePosition = newPos;

        auto newAzimuth =  calculateAzimuth();
        auto newElevation =  calculateElevation();

        listeners.call([newAzimuth, newElevation](Listener& l) { l.pannerChanged(newAzimuth, newElevation); });

        repaint();
    }
}

void PannerVisualisation::mouseUp(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    dragging = false;
    mouseIsActive = false;
}

bool PannerVisualisation::isInsideSmallCircle(const juce::Point<float> &point) {
    return smallCirclePosition.getDistanceFrom(point) <= 50.0;
}

float PannerVisualisation::calculateAzimuth() const {
    auto bounds = getLocalBounds().reduced(getWidth() / reductionDivide);
    auto center = bounds.getCentre().toFloat();
    float angle = std::atan2(smallCirclePosition.y - center.y, smallCirclePosition.x - center.x) * (180 / juce::MathConstants<float>::pi);

    angle -= 270;

    if (angle > 180) {
        angle -= 360;
    } else if (angle < -180) {
        angle += 360;
    }

    return angle;
}

float PannerVisualisation::calculateElevation() const {
    auto bounds = getLocalBounds().reduced(getWidth() / reductionDivide);
    auto center = bounds.getCentre().toFloat();
    float radius = static_cast<float>(bounds.getWidth() / 2.0);

    float distance = smallCirclePosition.getDistanceFrom(center);
    float elevation = 90 - (distance / radius) * 90;

    return elevation;
}


void PannerVisualisation::setAzimuthAndElevation(float azimuth, float elevation) {
    if (mouseIsActive || (azimuth == lastAzimuth && elevation == lastElevation)) {
        return;
    }

    lastAzimuth = azimuth;
    lastElevation = elevation;

    // TODO Bug at 180 / -180
    azimuth = std::clamp(azimuth, -179.0f, 179.0f);
    elevation = std::clamp(elevation, 0.0f, 90.0f);

    if (azimuth < 0) {
        azimuth += 360;
    }

    float azimuthRadians = (90.0f - azimuth) * (juce::MathConstants<float>::pi / 180.0f);

    if (azimuthRadians < 0) {
        azimuthRadians += 2 * juce::MathConstants<float>::pi;
    }

    auto bounds = getLocalBounds().reduced(getWidth() / reductionDivide);
    auto center = bounds.getCentre().toFloat();
    float outerRadius = static_cast<float>(bounds.getWidth() / 2.0);

    float radius = outerRadius * (1.0f - (elevation / 90.0f));

    float newX = center.x + std::cos(azimuthRadians) * radius;
    float newY = center.y - std::sin(azimuthRadians) * radius;
    smallCirclePosition.setXY(newX, newY);

    repaint();
}

void PannerVisualisation::addListener(PannerVisualisation::Listener *listenerToAdd) {
    listeners.add(listenerToAdd);
}

void PannerVisualisation::removeListener(PannerVisualisation::Listener *listenerToRemove) {
    listeners.remove(listenerToRemove);
}
