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
            Z_ID = {"param_z", 1};

    

    inline static const juce::String
            AZIM_NAME = "Azimuth",
            ELEV_NAME = "Elevation",
            DIST_NAME = "Distance",
            X_NAME = "X",
            Y_NAME = "Y",
            Z_NAME = "Z";
    

    static juce::StringArray getPluginParameterList();
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    const inline static float defaultAzimParam { 0.f };
    const inline static float defaultElevParam { 0.f };
    const inline static float defaultDistParam { 0.f };
    const inline static float defaultXParam { 0.f };
    const inline static float defaultYParam { 0.f };
    const inline static float defaultZParam { 0.f };

    inline static juce::NormalisableRange<float> azimRange {-180.0f, 180.0f, 1.0f},
                                                elevRange {-90.0f, 90.0f, 1.0f},
                                                distRange {0.0f, 17.4f, 0.1f},
                                                xRange {-10.f, 10.f, 0.1f},
                                                yRange {-10.f, 10.f, 0.1f},
                                                zRange {-10.f, 10.f, 0.1f};


private:
    inline static juce::StringArray parameterList;


    inline static juce::AudioParameterFloatAttributes percentage_attributes = juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float x, auto) { return juce::String (x * 100.f, 0); })
            .withValueFromStringFunction ([] (const juce::String& x) { return x.getFloatValue() / 100; })
            .withLabel ("%");

    JUCE_HEAVYWEIGHT_LEAK_DETECTOR (PluginParameters)
};

class ParameterListener : public juce::AudioProcessorValueTreeState::Listener
{
public:
    ParameterListener(juce::AudioProcessorValueTreeState& state, const juce::String& parameterID);
    ~ParameterListener() override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void updateCartesianCoordinates();
    void updateSphericalCoordinates();
    void boundRadius(float radius);
private:
    juce::AudioProcessorValueTreeState& treeState;
    juce::String paramID;
};


class UpdateManager
// singleton class to manage update flags and cached values
{
public:
    static UpdateManager& getInstance()
    {
        static UpdateManager instance; 
        return instance;
    }

    UpdateManager(const UpdateManager&) = delete;
    void operator=(const UpdateManager&) = delete;

    void setIsUpdating(bool value) { isUpdating = value; }
    bool getIsUpdating() const { return isUpdating; }

    void setIsUpdatingCartesian(bool value) { isUpdatingCartesian = value; }
    bool getIsUpdatingCartesian() const { return isUpdatingCartesian; }

    void setIsUpdatingSpherical(bool value) { isUpdatingSpherical = value; }
    bool getIsUpdatingSpherical() const { return isUpdatingSpherical; }

    void setOldRadius(float radius) { oldRadius = radius; }
    float getOldRadius() const { return oldRadius; }

private:
    UpdateManager() : isUpdating(false), isUpdatingCartesian(false), isUpdatingSpherical(false), oldRadius(0.0f) {} 

    bool isUpdating;
    bool isUpdatingCartesian;
    bool isUpdatingSpherical;

    float oldRadius;    
};

#endif //BINAURALPANNER_PLUGINPARAMETERS_H
