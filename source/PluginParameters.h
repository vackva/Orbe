#ifndef BINAURALPANNER_PLUGINPARAMETERS_H
#define BINAURALPANNER_PLUGINPARAMETERS_H

#include <JuceHeader.h>

class PluginParameters {
public:
    inline static const juce::ParameterID
            AZIM_ID = {"param_azim", 1},
            ELEV_ID = {"param_elev", 2},
            DIST_ID = {"param_dist", 3},
            DRY_WET_ID = {"param_mix", 1}
    ;

    inline static const juce::String
            AZIM_NAME = "Azimuth",
            ELEV_NAME = "Elevation",
            DIST_NAME = "Distance",
            DRY_WET_NAME = "Global Mix"
    ;

    static juce::StringArray getPluginParameterList();
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    inline static juce::StringArray parameterList;
    inline static juce::NormalisableRange<float> azimRange {-180.0f, 180.0f, 1.0f},
                                                 elevRange {-90.0f, 90.0f, 1.0f},
                                                 distRange {1.0f, 100.0f, 0.1f},
                                                 dryWetRange {0.0f, 1.0f, 0.01f};

    inline static juce::AudioParameterFloatAttributes percentage_attributes = juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float x, auto) { return juce::String (x * 100.f, 0); })
            .withValueFromStringFunction ([] (const juce::String& x) { return x.getFloatValue() / 100; })
            .withLabel ("%");

    JUCE_HEAVYWEIGHT_LEAK_DETECTOR (PluginParameters)
};


#endif //BINAURALPANNER_PLUGINPARAMETERS_H
