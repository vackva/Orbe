#include "PluginParameters.h"

juce::AudioProcessorValueTreeState::ParameterLayout PluginParameters::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(AZIM_ID,
                                                                 AZIM_NAME,
                                                                 azimRange,
                                                                 0.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ELEV_ID,
                                                                 ELEV_NAME,
                                                                 elevRange,
                                                                 0.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(DIST_ID,
                                                                 DIST_NAME,
                                                                 distRange,
                                                                 1.f));

    params.push_back( std::make_unique<juce::AudioParameterFloat>(DRY_WET_ID,
                                                                  DRY_WET_NAME,
                                                                  dryWetRange,
                                                                  1.f,
                                                                  percentage_attributes));

    for (const auto & param : params) {
        parameterList.add(param->getParameterID());
    }

    return { params.begin(), params.end() };
}

juce::StringArray PluginParameters::getPluginParameterList() {
    return parameterList;
}