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

    void prepare(const juce::dsp::ProcessSpec spec);
    bool submitJob(float azm, float elev);

    void hrirAccessed ();

    juce::AudioBuffer<float>& getHRIR();

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
    juce::AudioBuffer<float> hrirBuffer;
};


#endif //BINAURALPANNER_HRIRLOADER_H
