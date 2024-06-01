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
    
    currentConvolution.prepare(processSpec);
    previousConvolution.prepare(processSpec);
    
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

    // copy avoiding allocation -> check if really no memory gets allocated
    bufferCopy.makeCopyOf(buffer, true);

    if (hrirAvailable.load()) {
        updateHRIR();
    }

    if (hrirRequestDenied) {
        hrirRequestDenied = false;
        requestNewHRIR();
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> blockCopy(bufferCopy);
    
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::ProcessContextReplacing<float> contextCopy(blockCopy);
    
    if (convolutionReady) {
        currentConvolution.process(context);
        previousConvolution.process(contextCopy);
    }
    
    // mix buffers convolved with current and previous hrir
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        buffer.addFrom(channel, 0, bufferCopy, channel, 0, buffer.getNumSamples());
    }
    
    buffer.applyGain(0.5);

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
    
    currentConvolution.loadImpulseResponse(std::move(hrirLoader.getCurrentHRIR()), getSampleRate(), dsp::Convolution::Stereo::yes, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    previousConvolution.loadImpulseResponse(std::move(hrirLoader.getPreviousHRIR()), getSampleRate(), dsp::Convolution::Stereo::yes, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);

    hrirLoader.hrirAccessed();
    convolutionReady = true;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
