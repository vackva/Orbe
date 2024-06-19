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
       parameters (*this, nullptr, juce::Identifier (ProjectInfo::projectName), PluginParameters::createParameterLayout()),
       parameterListener(parameters)

{
    
    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.addParameterListener(parameterID, this);
    }

    sofaChoiceParam = dynamic_cast<juce::AudioParameterChoice*> ( parameters.getParameter( PluginParameters::SOFA_CHOICE_ID.getParamID() ) );
    sofaChoices hrirChoice = static_cast<sofaChoices> ( sofaChoiceParam->getIndex() );
    
    paramAzimuth.store(PluginParameters::defaultAzimParam);
    paramElevation.store(PluginParameters::defaultElevParam);
    paramDistance.store(PluginParameters::defaultDistParam);

    paramX.store(PluginParameters::defaultXParam);
    paramY.store(PluginParameters::defaultYParam);
    paramZ.store(PluginParameters::defaultZParam);

    paramLFOStart.store(PluginParameters::defaultLFOStartParam);
    paramXLFORate.store(PluginParameters::defaultXLFORateParam);
    paramXLFODepth.store(PluginParameters::defaultXLFODepthParam);
    paramXLFOPhase.store(PluginParameters::defaultXLFOPhaseParam);
    paramXLFOOffset.store(PluginParameters::defaultXLFOOffsetParam);
    paramYLFORate.store(PluginParameters::defaultYLFORateParam);
    paramYLFODepth.store(PluginParameters::defaultYLFODepthParam);
    paramYLFOPhase.store(PluginParameters::defaultYLFOPhaseParam);
    paramYLFOOffset.store(PluginParameters::defaultYLFOOffsetParam);
    paramZLFORate.store(PluginParameters::defaultZLFORateParam);
    paramZLFODepth.store(PluginParameters::defaultZLFODepthParam);
    paramZLFOPhase.store(PluginParameters::defaultZLFOPhaseParam);
    paramZLFOOffset.store(PluginParameters::defaultZLFOOffsetParam);


    xLFO = std::make_unique<juce::dsp::Oscillator<float>>();
    yLFO = std::make_unique<juce::dsp::Oscillator<float>>();
    zLFO = std::make_unique<juce::dsp::Oscillator<float>>();

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
    
    convolution.prepare(processSpec);    
    
    int numDelayChannels = 1;
    dsp::ProcessSpec delaySpec{sampleRate,
                            (juce::uint32) samplesPerBlock,
                            (juce::uint32) numDelayChannels };
    
    delayLineLeft.prepare(delaySpec);
    delayLineRight.prepare(delaySpec);
    
    smoothDelayLeft.reset( sampleRate, 0.1 );
    smoothDelayRight.reset( sampleRate, 0.1 );
    
    float maxDelayInSamples = sampleRate * 2;
    delayLineLeft.setMaximumDelayInSamples( maxDelayInSamples );
    delayLineRight.setMaximumDelayInSamples( maxDelayInSamples );
    

    convolutionReady = false;
    
    requestNewHRIR( );

    xLFO->prepare(juce::dsp::ProcessSpec({ getSampleRate() / getBlockSize(), (juce::uint32)getBlockSize(), 1 }));
    xLFO->setFrequency(0.f);
    xLFO->initialise([](float x) { return x;}, 128);
    yLFO->prepare(juce::dsp::ProcessSpec({ getSampleRate() / getBlockSize(), (juce::uint32)getBlockSize(), 1 }));
    yLFO->setFrequency(0.f);
    yLFO->initialise([](float x) { return x;}, 128);
    zLFO->prepare(juce::dsp::ProcessSpec({ getSampleRate() / getBlockSize(), (juce::uint32)getBlockSize(), 1 }));
    zLFO->setFrequency(0.f);
    zLFO->initialise([](float x) { return x;}, 128);

}

void AudioPluginAudioProcessor::releaseResources()
{
    xLFO->reset();
    yLFO->reset();
    zLFO->reset();
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
    

    if (*parameters.getRawParameterValue("param_lfo_start") > 0.5f)
    {
        processLFOs();
    }


    // UPDATE HRIR

    if (hrirAvailable.load()) {
        updateHRIR();
    }

    if (hrirRequestDenied) {
        hrirRequestDenied = false;
        requestNewHRIR();
    }
    // MAKE SIGNAL MONO

    buffer.addFrom(0, 0, buffer.getReadPointer(1), buffer.getNumSamples());
    buffer.applyGain(0.5);
    buffer.copyFrom(1, 0, buffer.getReadPointer(0), buffer.getNumSamples());

    // APPLY CONVOLUTION
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    

    if ( convolutionReady) 
    {
        convolution.process( context );
    }
    
    // Apply Distance Compensation
    float distance = paramDistance.load();
    float distanceGain =  0.2 / (jmax(0.0f, distance) + 1);
    buffer.applyGainRamp(0, buffer.getNumSamples(), lastDistanceGain, distanceGain);
    lastDistanceGain = distanceGain;

    // APPLY DELAY   
    if (true) { // dopplereffect enabled
        float doppler_delay = distance / 343 * getSampleRate(); 
        smoothDelayLeft.setTargetValue( delayTimeLeft + doppler_delay);
        smoothDelayRight.setTargetValue( delayTimeRight + doppler_delay );
    } else {
        smoothDelayLeft.setTargetValue( delayTimeLeft );
        smoothDelayRight.setTargetValue( delayTimeRight );
    }


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
    if (parameterID == PluginParameters::PRESETS_ID.getParamID()) {
        int selectedOption = static_cast<int>(newValue);
        applyPreset(selectedOption);
    }
    // Reset LFOs if rate is changed to realign phase relationship
    if (parameterID == PluginParameters::XLFO_RATE_ID.getParamID() ||
        parameterID == PluginParameters::YLFO_RATE_ID.getParamID() ||
        parameterID == PluginParameters::ZLFO_RATE_ID.getParamID()) {
        refreshLFOs();
    }
    
    // Change hrir if sofa choice parameter changed
    if (parameterID == PluginParameters::SOFA_CHOICE_ID.getParamID() )
    {
        hrirLoader.sofaChoice = static_cast<sofaChoices> ( sofaChoiceParam->getIndex() );
        requestNewHRIR();
    }
    
    if ( parameterID == PluginParameters::INTERP_ID.getParamID() )
    {
        hrirLoader.doNearestNeighbourInterpolation = newValue;
    }
    
    parameterListener.parameterChanged(parameterID, newValue);
}





float AudioPluginAudioProcessor::getAtomicParameterValue(const String &parameterID) {
    if (parameterID == PluginParameters::AZIM_ID.getParamID()) {
        return paramAzimuth.load();
    }
    else if (parameterID == PluginParameters::ELEV_ID.getParamID()) {
        return paramElevation.load();
    }
    else if (parameterID == PluginParameters::DIST_ID.getParamID()) {
        return paramDistance.load();
    }
    else {
        return 0.0f;
    }

}

juce::AudioProcessorValueTreeState &AudioPluginAudioProcessor::getValueTreeState() {
    return parameters;
}

void AudioPluginAudioProcessor::updateHRIR() {
    // DBG("updateHRIR() wurde aufgerufen.");

    hrirAvailable.store(false);
    
    convolution.loadImpulseResponse(std::move(hrirLoader.getCurrentHRIR()), getSampleRate(), custom_juce::Convolution::Stereo::yes, custom_juce::Convolution::Trim::no, custom_juce::Convolution::Normalise::no);
    hrirLoader.getCurrentDelays(delayTimeLeft, delayTimeRight);
    
    hrirLoader.hrirAccessed();
    convolutionReady = true;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}


//==============================================================================
// LFO Methods


void AudioPluginAudioProcessor::processLFOs() 
{
    if (*parameters.getRawParameterValue("param_lfo_start") > 0.5f) 
    {
        // X LFO
        if ((*parameters.getRawParameterValue("param_xlfo_rate") > 0.0f) && (*parameters.getRawParameterValue("param_xlfo_depth") > 0.0f))
        {
            float frequency = *parameters.getRawParameterValue("param_xlfo_rate");
            float phase = *parameters.getRawParameterValue("param_xlfo_phase");
            float amplitude = *parameters.getRawParameterValue("param_xlfo_depth") / 10.0f;
            float offset = *parameters.getRawParameterValue("param_xlfo_offset");

            xLFO->setFrequency(frequency);
            float xlfoSample = amplitude * std::sin(xLFO->processSample(degreesToRadians(phase))) + offset;

            xlfoSample = juce::jlimit(-10.0f, 10.0f, xlfoSample);
            float normalizedX = PluginParameters::xRange.convertTo0to1(xlfoSample);
            parameters.getParameter("param_x")->setValueNotifyingHost(normalizedX);
        }
        // Y LFO
        if ((*parameters.getRawParameterValue("param_ylfo_rate") > 0.0f) && (*parameters.getRawParameterValue("param_ylfo_depth") > 0.0f))
        {
            float frequency = *parameters.getRawParameterValue("param_ylfo_rate");
            float phase = *parameters.getRawParameterValue("param_ylfo_phase");
            float amplitude = *parameters.getRawParameterValue("param_ylfo_depth") / 10.0f;
            float offset = *parameters.getRawParameterValue("param_ylfo_offset");

            yLFO->setFrequency(frequency);
            float ylfoSample = amplitude * std::sin(yLFO->processSample(degreesToRadians(phase))) + offset;

            ylfoSample = juce::jlimit(-10.0f, 10.0f, ylfoSample);
            float normalizedY = PluginParameters::yRange.convertTo0to1(ylfoSample);
            parameters.getParameter("param_y")->setValueNotifyingHost(normalizedY);
        }
        // Z LFO
        if ((*parameters.getRawParameterValue("param_zlfo_rate") > 0.0f) && (*parameters.getRawParameterValue("param_zlfo_depth") > 0.0f))
        {
            float frequency = *parameters.getRawParameterValue("param_zlfo_rate");
            float phase = *parameters.getRawParameterValue("param_zlfo_phase");
            float amplitude = *parameters.getRawParameterValue("param_zlfo_depth") / 10.0f;
            float offset = *parameters.getRawParameterValue("param_zlfo_offset");

            zLFO->setFrequency(frequency);
            float zlfoSample = amplitude * std::sin(zLFO->processSample(degreesToRadians(phase))) + offset;

            zlfoSample = juce::jlimit(-10.0f, 10.0f, zlfoSample);
            float normalizedZ = PluginParameters::zRange.convertTo0to1(zlfoSample);
            parameters.getParameter("param_z")->setValueNotifyingHost(normalizedZ);
        }
    }
}

void AudioPluginAudioProcessor::refreshLFOs() 
{
    xLFO->reset();
    yLFO->reset();
    zLFO->reset();
    xLFO->initialise([](float x) { return x;}, 128);
    yLFO->initialise([](float x) { return x;}, 128);
    zLFO->initialise([](float x) { return x;}, 128);
}



void AudioPluginAudioProcessor::applyPreset(int presetOption) 
{
    switch (presetOption) {
        case 0: // Custom
            // Benutzerdefinierte Parameter, nichts zu tun
            break;
        case 1: // Top View: Great Circle
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 100.f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 100.f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break; 
        case 2: // Top View: Eight Figure
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.3f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 100.f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.6f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 50.f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;
        case 3: // Top View: 3D Infinity 
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.6f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 50.f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.3f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 100.f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.6f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 100.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;
        case 4: //  Front View: Diagonal Circle
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 33.3f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 66.6f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 99.9f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true; 
                break;
        case 5: // Top View: Diagonal Eight
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.3f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 100.f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.6f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 50.f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.3f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 100.f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;
        case 6: // Top View: Sparse Spiral
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.2f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 100.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.6f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 100.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.6f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 100.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true; 
                break;
        case 7: // Top View: Dense Spiral
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.1f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 100.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 1.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 28.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 1.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 100.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = -180.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;
        case 8: // Top View: 3D Horsehoe
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.4f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 75.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.2f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 75.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.4f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 100.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = -90.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;
        case 9: // Ping Pong
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.1f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 27.3f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 6.5f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.9f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 44.2f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true; 
                break;
        case 10: // Top View: Small Circle - Front Left
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 35.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 5.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 35.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 5.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;
        case 11: // Top View: Small Circle - Front Right
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 35.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = 5.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 35.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = -5.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;
        case 12: // Top View: Small Circle - Back Left
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 35.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = -5.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 35.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = 5.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;
        case 13: // Top View: Small Circle - Back Right 
                parameters.getParameterAsValue(PluginParameters::XLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::XLFO_DEPTH_ID.getParamID()) = 35.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::XLFO_OFFSET_ID.getParamID()) = -5.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_RATE_ID.getParamID()) = 0.5f;
                parameters.getParameterAsValue(PluginParameters::YLFO_DEPTH_ID.getParamID()) = 35.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_PHASE_ID.getParamID()) = 90.0f;
                parameters.getParameterAsValue(PluginParameters::YLFO_OFFSET_ID.getParamID()) = -5.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_RATE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_DEPTH_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_PHASE_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::ZLFO_OFFSET_ID.getParamID()) = 0.0f;
                parameters.getParameterAsValue(PluginParameters::LFO_START_ID.getParamID()) = true;
                break;        
    }
}
