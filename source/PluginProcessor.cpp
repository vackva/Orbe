#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       parameters (*this, nullptr, juce::Identifier (ProjectInfo::projectName), PluginParameters::createParameterLayout())
{
    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.addParameterListener(parameterID, this);
    }

    paramAzimuth.store(PluginParameters::defaultAzimParam);
    paramElevation.store(PluginParameters::defaultElevParam);
    paramDistance.store(PluginParameters::defaultDistParam);

    hrirLoader.newHRIRAvailable = [this] () {
        hrirAvailable.store(true);
    };
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.removeParameterListener(parameterID, this);
    }
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    dsp::ProcessSpec processSpec {sampleRate,
                                  (juce::uint32) samplesPerBlock,
                                  (juce::uint32) getTotalNumInputChannels() };

    hrirLoader.prepare(processSpec);
    
    //currentConvolution.prepare(processSpec);
    //previousConvolution.prepare(processSpec);
    
    convA.prepare(processSpec);
    convB.prepare(processSpec);
    
    crossoverFactor.reset( samplesPerBlock );
    
    int numDelayChannels = 1;
    dsp::ProcessSpec delaySpec{sampleRate,
                            (juce::uint32) samplesPerBlock,
                            (juce::uint32) numDelayChannels };
    
    delayLineLeft.prepare(delaySpec);
    delayLineRight.prepare(delaySpec);
    
    smoothDelayLeft.reset( samplesPerBlock );
    smoothDelayRight.reset( samplesPerBlock );
    
    float maxDelayInSamples = sampleRate * 2;
    delayLineLeft.setMaximumDelayInSamples( maxDelayInSamples );
    delayLineRight.setMaximumDelayInSamples( maxDelayInSamples );
    
    bufferCopy.setSize(processSpec.numChannels, processSpec.maximumBlockSize);

    convolutionReady = false;

    requestNewHRIR();
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Check if both the input and output layouts are stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo() ||
        layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }

    return true;
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    // UPDATE HRIR

    if (hrirAvailable.load()) {
        updateHRIR();
    }

    if (hrirRequestDenied) {
        hrirRequestDenied = false;
        requestNewHRIR();
    }
    
    // APPLY CONVOLUTION
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    

    if ( convolutionReady) 
    {
        convA.process( context );
    }
    
    // APPLY DELAY    
    smoothDelayLeft.setTargetValue( delayTimeLeft );
    smoothDelayRight.setTargetValue( delayTimeRight );

    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            delayLineLeft.setDelay( smoothDelayLeft.getNextValue() );
            delayLineRight.setDelay( smoothDelayRight.getNextValue() );

            delayLineLeft.pushSample(0, buffer.getSample(0, sample));
            delayLineRight.pushSample(0, buffer.getSample(1, sample));

            buffer.setSample(0, sample, delayLineLeft.popSample(0));
            buffer.setSample(1, sample, delayLineRight.popSample(0));
        }
    
    buffer.applyGain(0.5);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

void AudioPluginAudioProcessor::parameterChanged(const String &parameterID, float newValue) {
    if (parameterID == PluginParameters::AZIM_ID.getParamID()) {
        paramAzimuth.store(newValue);
        requestNewHRIR();
    } else if (parameterID == PluginParameters::ELEV_ID.getParamID()) {
        paramElevation.store(newValue);
        requestNewHRIR();
    } else if (parameterID == PluginParameters::DIST_ID.getParamID()) {
        paramDistance.store(newValue);
    }
}

float AudioPluginAudioProcessor::getAtomicParameterValue(const String &parameterID) {
    if (parameterID == PluginParameters::AZIM_ID.getParamID()) {
        return paramAzimuth.load();
    }
    else if (parameterID == PluginParameters::ELEV_ID.getParamID()) {
        return paramElevation.load();
    }
    else {
        return 0.0f;
    }
}

juce::AudioProcessorValueTreeState &AudioPluginAudioProcessor::getValueTreeState() {
    return parameters;
}

void AudioPluginAudioProcessor::updateHRIR() {
    hrirAvailable.store(false);
    
    convA.loadImpulseResponse(std::move(hrirLoader.getCurrentHRIR()), 44100, dsp::Convolution::Stereo::yes, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    hrirLoader.getCurrentDelays(delayTimeLeft, delayTimeRight, getSampleRate());
    
    hrirLoader.hrirAccessed();
    convolutionReady = true;
    hrirChanged = true;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
