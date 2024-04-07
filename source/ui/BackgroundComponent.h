#ifndef BINAURALPANNER_BACKGROUNDCOMPONENT_H
#define BINAURALPANNER_BACKGROUNDCOMPONENT_H

#include <JuceHeader.h>

class BackgroundComponent : public juce::Component {
    void paint (juce::Graphics& g) override;

private:
    juce::Colour backgroundColour {0xff080809};
};

#endif //BINAURALPANNER_BACKGROUNDCOMPONENT_H
