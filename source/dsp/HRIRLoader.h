#ifndef BINAURALPANNER_HRIRLOADER_H
#define BINAURALPANNER_HRIRLOADER_H

#include <JuceHeader.h>
#include "SofaReader.h"

struct HRIRJob {
    std::atomic<float> azm;
    std::atomic<float> elev;
};

class HRIRLoader : public juce::Thread {
public:
    HRIRLoader();
    ~HRIRLoader();

    void prepare(const juce::dsp::ProcessSpec spec, hrirChoices hrirChoice);
    bool submitJob(float azm, float elev);

    void hrirAccessed ();

    juce::AudioBuffer<float>& getCurrentHRIR();
    void getCurrentDelays(float &left, float &right);
    //juce::AudioBuffer<float>& getPreviousHRIR();

    // TODO replace with Listener
    std::function<void()> newHRIRAvailable;

private:
    void run() override;

private:
    std::atomic<bool> jobSubmitted {false};
    std::atomic<bool> hrirFinished {true};

    SofaReader sofaReader;
    juce::dsp::ProcessSpec currentSpec;
    HRIRJob requestedHRIR;
    
    juce::AudioBuffer<float> currentHrirBuffer;
    float currentLeftDelay;
    float currentRightDelay;
    //juce::AudioBuffer<float> previousHrirBuffer;
    //juce::AudioBuffer<float> tempHrirBuffer;

};


#endif //BINAURALPANNER_HRIRLOADER_H
