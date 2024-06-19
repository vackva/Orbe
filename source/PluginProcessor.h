#pragma once

#include <JuceHeader.h>
#include "dsp/convolution/custom_juce_Convolution.h"

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

    ParameterListener parameterListener;

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void updateHRIR();
    void requestNewHRIR()
    {
        bool success = hrirLoader.submitJob(paramAzimuth.load(), paramElevation.load());
        hrirRequestDenied = !success;
    }
    void applyPreset(int presetOption);
    void processLFOs();
    void refreshLFOs();

private:
    juce::AudioProcessorValueTreeState parameters;

    HRIRLoader hrirLoader;
    
    juce::AudioParameterChoice* sofaChoiceParam;
    juce::AudioParameterBool* interpParam;

    bool hrirRequestDenied = false;
    std::atomic<bool> hrirAvailable { false };
    bool convolutionReady = false;

    std::unique_ptr<juce::dsp::Oscillator<float>> xLFO;
    std::unique_ptr<juce::dsp::Oscillator<float>> yLFO;
    std::unique_ptr<juce::dsp::Oscillator<float>> zLFO;

    std::atomic<float> paramAzimuth { 0.0f };
    std::atomic<float> paramElevation { 0.0f };
    std::atomic<float> paramDistance { 0.0f };


    custom_juce::Convolution convolution;
    
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLineLeft;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLineRight;

    float delayTimeLeft = 0;
    float delayTimeRight = 0;
    
    juce::SmoothedValue<float> smoothDelayLeft { 0.0f };
    juce::SmoothedValue<float> smoothDelayRight { 0.0f };
    
    
    std::atomic<float> paramX { 0.0f };
    std::atomic<float> paramY { 0.0f };
    std::atomic<float> paramZ { 0.0f };
    std::atomic<bool> paramLFOStart { false };
    std::atomic<float> paramXLFORate { 0.0f };
    std::atomic<float> paramXLFODepth { 0.0f };
    std::atomic<float> paramXLFOPhase { 0.0f };
    std::atomic<float> paramXLFOOffset { 0.0f };
    std::atomic<float> paramYLFORate { 0.0f };
    std::atomic<float> paramYLFODepth { 0.0f };
    std::atomic<float> paramYLFOPhase { 0.0f };
    std::atomic<float> paramYLFOOffset { 0.0f };
    std::atomic<float> paramZLFORate { 0.0f };
    std::atomic<float> paramZLFODepth { 0.0f };
    std::atomic<float> paramZLFOPhase { 0.0f };
    std::atomic<float> paramZLFOOffset { 0.0f };


    float lastDistanceGain = 0.0f;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
