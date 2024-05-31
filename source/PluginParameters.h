#ifndef BINAURALPANNER_PLUGINPARAMETERS_H
#define BINAURALPANNER_PLUGINPARAMETERS_H

#include <JuceHeader.h>

class PluginParameters {
public:
    inline static const juce::ParameterID
            AZIM_ID = {"param_azim", 1},
            ELEV_ID = {"param_elev", 1},
            DIST_ID = {"param_dist", 1},
            X_ID = {"param_x", 1},
            Y_ID = {"param_y", 1},
            Z_ID = {"param_z", 1},
            DRY_WET_ID = {"param_mix", 1}
    ;

    inline static const juce::String
            AZIM_NAME = "Azimuth",
            ELEV_NAME = "Elevation",
            DIST_NAME = "Distance",
            X_NAME = {"X", 1},
            Y_NAME = {"Y", 1},
            Z_NAME = {"Z", 1},
            DRY_WET_NAME = "Global Mix"
    ;

    static juce::StringArray getPluginParameterList();
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    const inline static float defaultAzimParam { 0.f };
    const inline static float defaultElevParam { 90.f };
    const inline static float defaultDistParam { 1.f };
    const inline static float defaultDryWetParam { 1.f };

private:
    inline static juce::StringArray parameterList;
    inline static juce::NormalisableRange<float> azimRange {-180.0f, 180.0f, 1.0f},
                                                 elevRange {0.f, 90.0f, 1.0f},
                                                 distRange {1.0f, 100.0f, 0.1f},
                                                 dryWetRange {0.0f, 1.0f, 0.01f},
                                                 xRange {-10.f, 10.f, 0.1f};

    inline static juce::AudioParameterFloatAttributes percentage_attributes = juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float x, auto) { return juce::String (x * 100.f, 0); })
            .withValueFromStringFunction ([] (const juce::String& x) { return x.getFloatValue() / 100; })
            .withLabel ("%");

    JUCE_HEAVYWEIGHT_LEAK_DETECTOR (PluginParameters)
};


#endif //BINAURALPANNER_PLUGINPARAMETERS_H
