#include "PluginParameters.h"
#include "Constants.h"

juce::AudioProcessorValueTreeState::ParameterLayout PluginParameters::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(AZIM_ID,
                                                                 AZIM_NAME,
                                                                 azimRange,
                                                                 defaultAzimParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ELEV_ID,
                                                                 ELEV_NAME,
                                                                 elevRange,
                                                                 defaultElevParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(DIST_ID,
                                                                 DIST_NAME,
                                                                 distRange,
                                                                 defaultDistParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(X_ID,
                                                                 X_NAME,
                                                                 xRange,
                                                                  defaultXParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(Y_ID,
                                                                 Y_NAME,
                                                                 yRange,
                                                                 defaultYParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(Z_ID,
                                                                 Z_NAME,
                                                                 zRange,
                                                                 defaultZParam));

    params.push_back(std::make_unique<juce::AudioParameterBool>(LFO_START_ID,
                                                                LFO_START_NAME,
                                                                defaultLFOStartParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(XLFO_RATE_ID,
                                                                 XLFO_RATE_NAME,
                                                                 xLFORateRange,
                                                                 defaultXLFORateParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(XLFO_DEPTH_ID,
                                                                 XLFO_DEPTH_NAME,
                                                                 xLFODepthRange,
                                                                 defaultXLFODepthParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(XLFO_PHASE_ID,
                                                                 XLFO_PHASE_NAME,
                                                                 xLFOPhaseRange,
                                                                 defaultXLFOPhaseParam));
                                                            
    params.push_back(std::make_unique<juce::AudioParameterFloat>(XLFO_OFFSET_ID,
                                                                 XLFO_OFFSET_NAME,
                                                                 xLFOOffsetRange,
                                                                 defaultXLFOOffsetParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(YLFO_RATE_ID,
                                                                YLFO_RATE_NAME,
                                                                yLFORateRange,
                                                                defaultYLFORateParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(YLFO_DEPTH_ID,
                                                                YLFO_DEPTH_NAME,
                                                                yLFODepthRange,
                                                                defaultYLFODepthParam));
                                                            
    params.push_back(std::make_unique<juce::AudioParameterFloat>(YLFO_PHASE_ID,
                                                                YLFO_PHASE_NAME,
                                                                yLFOPhaseRange,
                                                                defaultYLFOPhaseParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(YLFO_OFFSET_ID,
                                                                YLFO_OFFSET_NAME,
                                                                yLFOOffsetRange,
                                                                defaultYLFOOffsetParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ZLFO_RATE_ID,
                                                                ZLFO_RATE_NAME,
                                                                zLFORateRange,
                                                                defaultZLFORateParam)); 

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ZLFO_DEPTH_ID,
                                                                ZLFO_DEPTH_NAME,
                                                                zLFODepthRange,
                                                                defaultZLFODepthParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ZLFO_PHASE_ID,
                                                                ZLFO_PHASE_NAME,
                                                                zLFOPhaseRange,
                                                                defaultZLFOPhaseParam));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ZLFO_OFFSET_ID,
                                                                ZLFO_OFFSET_NAME,
                                                                zLFOOffsetRange,
                                                                defaultZLFOOffsetParam));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(PRESETS_ID,
                                                                  PRESETS_NAME,
                                                                  juce::StringArray("Custom", "Great Circle", "Eight Figure", "3D Infinity", "Diagonal Circle", "Diagonal Eight", "Sparse Spiral", "Dense Spiral", "3D Horsehoe", "Ping Pong", "Small Circle Front Left", "Small Circle Front Right", "Small Circle Back Left", "Small Circle Back Right"),
                                                                  0));
                                                               

    

    for (const auto & param : params) {
        parameterList.add(param->getParameterID());
    }

    return { params.begin(), params.end() };
}

juce::StringArray PluginParameters::getPluginParameterList() {
    return parameterList;
}

ParameterListener::ParameterListener(juce::AudioProcessorValueTreeState& state)
    : treeState(state)
{

}

ParameterListener::~ParameterListener()
{

}


void ParameterListener::parameterChanged(const juce::String& parameterID, float newValue)
{

    // instance of the UpdateManager, which is a singleton class that manages the boolean flags for bidirectional parameter updates
    UpdateManager& updateManager = UpdateManager::getInstance();

    if (!updateManager.getIsUpdating())
    {
        updateManager.setIsUpdating(true);
        if (parameterID == PluginParameters::X_ID.getParamID() || parameterID == PluginParameters::Y_ID.getParamID() || parameterID == PluginParameters::Z_ID.getParamID())
        {
            // avoid endless recursion
            if (!updateManager.getIsUpdatingCartesian())
            {
                updateManager.setIsUpdatingSpherical(true);
                updateSphericalCoordinates();
                updateManager.setOldRadius(PluginParameters::distRange.convertFrom0to1(treeState.getParameter(PluginParameters::DIST_ID.getParamID())->getValue()));
                updateManager.setIsUpdatingSpherical(false);
            }
        }
        else if (parameterID == PluginParameters::DIST_ID.getParamID()|| parameterID == PluginParameters::AZIM_ID.getParamID() || parameterID == PluginParameters::ELEV_ID.getParamID())
        {
            // avoid endless recursion
            if (!updateManager.getIsUpdatingSpherical())
            {
                updateManager.setIsUpdatingCartesian(true);
                if (parameterID == PluginParameters::DIST_ID.getParamID())
                {
                    float radius = PluginParameters::distRange.convertFrom0to1(treeState.getParameter(PluginParameters::DIST_ID.getParamID())->getValue());
                    boundRadius(radius);
                    updateManager.setOldRadius(PluginParameters::distRange.convertFrom0to1(treeState.getParameter(PluginParameters::DIST_ID.getParamID())->getValue()));
                }
                if (parameterID == PluginParameters::AZIM_ID.getParamID() || parameterID == PluginParameters::ELEV_ID.getParamID())
                {
                    boundRadius(updateManager.getOldRadius());
                }
                updateCartesianCoordinates();
                updateManager.setIsUpdatingCartesian(false);
            }
        }
        updateManager.setIsUpdating(false);
    }
}

void ParameterListener::boundRadius(float radius) {
    // limit the radius to the boundaries of the quadratic room
    // get the current azimuth and elevation values
    float azimuthDeg = PluginParameters::azimRange.convertFrom0to1(treeState.getParameter(PluginParameters::AZIM_ID.getParamID())->getValue());
    float elevationDeg = PluginParameters::elevRange.convertFrom0to1(treeState.getParameter(PluginParameters::ELEV_ID.getParamID())->getValue());
    float elevationRad = degreesToRadians(elevationDeg);
    float azimuthRad = degreesToRadians(azimuthDeg);

    float colatitudeRad = juce::MathConstants<float>::halfPi - elevationRad;

    // direction vector for the line
    float dx = sin(colatitudeRad) * cos(azimuthRad);
    float dy = sin(colatitudeRad) * sin(azimuthRad);
    float dz = cos(colatitudeRad);

    // intersection of the line with the cube boundaries
    float tX = (!juce::approximatelyEqual(dx,0.0f)) ? (HALF_CUBE_EDGE_LENGTH / dx) : std::numeric_limits<float>::infinity();
    float tY = (!juce::approximatelyEqual(dy,0.0f)) ? (HALF_CUBE_EDGE_LENGTH / dy) : std::numeric_limits<float>::infinity();
    float tZ = (!juce::approximatelyEqual(dz,0.0f)) ? (HALF_CUBE_EDGE_LENGTH / dz) : std::numeric_limits<float>::infinity();

    // the intersection point is the closest one to the origin
    float tMin = std::min({std::abs(tX), std::abs(tY), std::abs(tZ)});
    // the maximum radius is the distance from the origin to the intersection point
    float maxRadius = tMin;

    // limit the new radius if it exceeds the room boundaries
    if (radius > maxRadius) {
        float normalizedRadius = PluginParameters::distRange.convertTo0to1(maxRadius);
        treeState.getParameter(PluginParameters::DIST_ID.getParamID())->setValueNotifyingHost(normalizedRadius);
    }
}

void ParameterListener::updateSphericalCoordinates()
{
    float normalizedX = treeState.getParameter("param_x")->getValue();
    float normalizedY = treeState.getParameter("param_y")->getValue();
    float normalizedZ = treeState.getParameter("param_z")->getValue();

    float x = PluginParameters::xRange.convertFrom0to1(normalizedX);
    float y = PluginParameters::yRange.convertFrom0to1(normalizedY);
    float z = PluginParameters::zRange.convertFrom0to1(normalizedZ);

    float radius = sqrt(x*x + y*y + z*z);
    float elevation;
    float azimuth;

    // update elevation
    if (juce::approximatelyEqual(radius, 0.0f)) {
        // avoid division by 0
        // design choice: keep the old elevation value (instead of setting it to 0)
        float oldNormalizedElevation= treeState.getParameter("param_elev")->getValue();
        float oldElevation = PluginParameters::elevRange.convertFrom0to1(oldNormalizedElevation);
        elevation = oldElevation;
    }
    else {
        elevation = juce::MathConstants<float>::halfPi - acos(z / radius); 
        elevation = radiansToDegrees(elevation); 
    }

    // update azimuth
    if (juce::approximatelyEqual(x, 0.0f) && juce::approximatelyEqual(y, 0.0f))
    {
        // avoid division by 0
        // design choice: keep the old azimuth value (instead of setting it to 0)
        float oldNormalizedAzimuth = treeState.getParameter("param_azim")->getValue();
        float oldAzimuth = PluginParameters::azimRange.convertFrom0to1(oldNormalizedAzimuth);
        azimuth = oldAzimuth;
    }
    else {
        // atan2 does (almost) all the work for us for the case distinctions
        azimuth = atan2(y, x);
        azimuth = radiansToDegrees(azimuth);
    }
    float normalizedRadius = PluginParameters::distRange.convertTo0to1(radius);
    float normalizedElevation = PluginParameters::elevRange.convertTo0to1(elevation);
    float normalizedAzimuth = PluginParameters::azimRange.convertTo0to1(azimuth);

    treeState.getParameter("param_dist")->setValueNotifyingHost(normalizedRadius);
    treeState.getParameter("param_elev")->setValueNotifyingHost(normalizedElevation);
    treeState.getParameter("param_azim")->setValueNotifyingHost(normalizedAzimuth);
}

void ParameterListener::updateCartesianCoordinates()
{
    float normalizedRadius = treeState.getParameter("param_dist")->getValue();
    float normalizedAzimuthDeg = treeState.getParameter("param_azim")->getValue();
    float normalizedElevationDeg = treeState.getParameter("param_elev")->getValue();

    float radius = PluginParameters::distRange.convertFrom0to1(normalizedRadius);
    float azimuthDeg = PluginParameters::azimRange.convertFrom0to1(normalizedAzimuthDeg);
    float elevationDeg = PluginParameters::elevRange.convertFrom0to1(normalizedElevationDeg);

    float colatitudeRad = juce::MathConstants<float>::halfPi - degreesToRadians(elevationDeg);
    float azimuthRad = degreesToRadians(azimuthDeg);

    float x = radius * sin(colatitudeRad) * cos(azimuthRad);
    float y = radius * sin(colatitudeRad) * sin(azimuthRad);
    float z = radius * cos(colatitudeRad);

    // always limit the values to the room boundaries
    x = jlimit(-HALF_CUBE_EDGE_LENGTH, HALF_CUBE_EDGE_LENGTH, x);
    y = jlimit(-HALF_CUBE_EDGE_LENGTH, HALF_CUBE_EDGE_LENGTH, y);
    z = jlimit(-HALF_CUBE_EDGE_LENGTH, HALF_CUBE_EDGE_LENGTH, z);

    float normalizedX = PluginParameters::xRange.convertTo0to1(x);
    float normalizedY = PluginParameters::yRange.convertTo0to1(y);
    float normalizedZ = PluginParameters::zRange.convertTo0to1(z);

    treeState.getParameter("param_x")->setValueNotifyingHost(normalizedX);
    treeState.getParameter("param_y")->setValueNotifyingHost(normalizedY);
    treeState.getParameter("param_z")->setValueNotifyingHost(normalizedZ);
}

