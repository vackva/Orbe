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
            LFO_START_ID = {"param_lfo_start", 1},
            XLFO_RATE_ID = {"param_xlfo_rate", 1},
            XLFO_DEPTH_ID = {"param_xlfo_depth", 1},
            XLFO_PHASE_ID = {"param_xlfo_phase", 1},
            XLFO_OFFSET_ID = {"param_xlfo_offset", 1},
            YLFO_RATE_ID = {"param_ylfo_rate", 1},
            YLFO_DEPTH_ID = {"param_ylfo_depth", 1},
            YLFO_PHASE_ID = {"param_ylfo_phase", 1},
            YLFO_OFFSET_ID = {"param_ylfo_offset", 1},
            ZLFO_RATE_ID = {"param_zlfo_rate", 1},
            ZLFO_DEPTH_ID = {"param_zlfo_depth", 1},
            ZLFO_PHASE_ID = {"param_zlfo_phase", 1},
            ZLFO_OFFSET_ID = {"param_zlfo_offset", 1},
            PRESETS_ID = {"param_presets", 1};
 

            

    inline static const juce::String
            AZIM_NAME = "Azimuth",
            ELEV_NAME = "Elevation",
            DIST_NAME = "Distance",
            X_NAME = "X",
            Y_NAME = "Y",
            Z_NAME = "Z",
            LFO_START_NAME = "Start",
            XLFO_RATE_NAME = "X LFO Rate",
            XLFO_DEPTH_NAME = "X LFO Depth",
            XLFO_PHASE_NAME = "X LFO Phase",
            XLFO_OFFSET_NAME = "X LFO Offset",
            YLFO_RATE_NAME = "Y LFO Rate",
            YLFO_DEPTH_NAME = "Y LFO Depth",
            YLFO_PHASE_NAME = "Y LFO Phase",
            YLFO_OFFSET_NAME = "Y LFO Offset",
            ZLFO_RATE_NAME = "Z LFO Rate",
            ZLFO_DEPTH_NAME = "Z LFO Depth",
            ZLFO_PHASE_NAME = "Z LFO Phase",
            ZLFO_OFFSET_NAME = "Z LFO Offset",
            PRESETS_NAME = "Presets"; 

            
    

    static juce::StringArray getPluginParameterList();
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    const inline static float defaultAzimParam { 0.f };
    const inline static float defaultElevParam { 0.f };
    const inline static float defaultDistParam { 0.f };
    const inline static float defaultXParam { 0.f };
    const inline static float defaultYParam { 0.f };
    const inline static float defaultZParam { 0.f };
    const inline static bool defaultLFOStartParam { false };
    const inline static float defaultXLFORateParam { 0.f };
    const inline static float defaultXLFODepthParam { 0.f };
    const inline static float defaultXLFOPhaseParam { 0.f };
    const inline static float defaultXLFOOffsetParam { 0.f };
    const inline static float defaultYLFORateParam { 0.f };
    const inline static float defaultYLFODepthParam { 0.f };
    const inline static float defaultYLFOPhaseParam { 0.f };
    const inline static float defaultYLFOOffsetParam { 0.f };
    const inline static float defaultZLFORateParam { 0.f };
    const inline static float defaultZLFODepthParam { 0.f };
    const inline static float defaultZLFOPhaseParam { 0.f };
    const inline static float defaultZLFOOffsetParam { 0.f };

    

    inline static juce::NormalisableRange<float> azimRange {-180.0f, 180.0f, 0.01f},
                                                elevRange {-90.0f, 90.0f, 0.01f},
                                                distRange {0.0f, 17.4f, 0.1f},
                                                xRange {-10.f, 10.f, 0.1f},
                                                yRange {-10.f, 10.f, 0.1f},
                                                zRange {-10.f, 10.f, 0.1f},
                                                xLFORateRange {0.0f, 1.5f, 0.1f},
                                                xLFODepthRange {0.0f, 100.f, 0.1f},
                                                xLFOPhaseRange {-180.f, 180.f, 1.f},
                                                xLFOOffsetRange {-10.f, 10.f, 0.1f},
                                                yLFORateRange {0.0f, 1.5f, 0.1f},
                                                yLFODepthRange {0.0f, 100.f, 0.1f},
                                                yLFOPhaseRange {-180.f, 180.f, 1.f},
                                                yLFOOffsetRange {-10.f, 10.f, 0.1f},
                                                zLFORateRange {0.0f, 1.5f, 0.1f},
                                                zLFODepthRange {0.0f, 100.f, 0.1f},
                                                zLFOPhaseRange {-180.f, 180.f, 1.f},
                                                zLFOOffsetRange {-10.f, 10.f, 0.1f};


private:
    inline static juce::StringArray parameterList;


    inline static juce::AudioParameterFloatAttributes percentage_attributes = juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float x, auto) { return juce::String (x * 100.f, 0); })
            .withValueFromStringFunction ([] (const juce::String& x) { return x.getFloatValue() / 100; })
            .withLabel ("%");

    JUCE_HEAVYWEIGHT_LEAK_DETECTOR (PluginParameters)
};

class ParameterListener 
{
public:
    ParameterListener(juce::AudioProcessorValueTreeState& state);
    ~ParameterListener();

    void parameterChanged(const juce::String& parameterID, float newValue);
    void updateCartesianCoordinates();
    void updateSphericalCoordinates();
    void boundRadius(float radius);
private:
    juce::AudioProcessorValueTreeState& treeState;
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
