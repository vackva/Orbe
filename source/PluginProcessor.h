#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

#include "PluginParameters.h"
#include "dsp/HRIRLoader.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor, private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    float getAtomicParameterValue(const juce::String& parameterID);
    juce::AudioProcessorValueTreeState& getValueTreeState();

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void updateHRIR();
    void requestNewHRIR() {
        bool success = hrirLoader.submitJob(paramAzimuth.load(), paramElevation.load());
        hrirRequestDenied = !success;
    }
private:
    juce::AudioProcessorValueTreeState parameters;

    //dsp::Convolution currentConvolution;
    //dsp::Convolution previousConvolution;
    
    juce::AudioBuffer<float> bufferCopy;

    HRIRLoader hrirLoader;

    bool hrirRequestDenied = false;
    std::atomic<bool> hrirAvailable { false };
    bool convolutionReady = false;
    bool hrirChanged = false;

    std::atomic<float> paramAzimuth { 0.0f };
    std::atomic<float> paramElevation { 0.0f };
    std::atomic<float> paramDistance { 0.0f };
    //std::atomic<bool> hrirChanged { false }; // not used?? -> if needed other member with same name has to be changed
    
    //enum activConv { activConvIsA, activConvIsB};
    bool activConvIsA = true;
    
    dsp::Convolution convA;
    dsp::Convolution convB;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
