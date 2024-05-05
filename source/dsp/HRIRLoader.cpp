#include "HRIRLoader.h"

HRIRLoader::HRIRLoader() : juce::Thread("HRIRLoader") {
}

HRIRLoader::~HRIRLoader() {
    stopThread(10);
}

void HRIRLoader::prepare(const juce::dsp::ProcessSpec spec) {
    stopThread(10);

    sofaReader.prepare(spec.sampleRate);
    currentSpec = spec;

    startThread(juce::Thread::Priority::high);
}

void HRIRLoader::run() {
    while (!threadShouldExit()) {
        if (jobSubmitted.load()) {
            jobSubmitted.store(false);

            hrirBuffer.setSize(currentSpec.numChannels, sofaReader.get_ir_length());
            sofaReader.get_hrirs(hrirBuffer, requestedHRIR.azm, requestedHRIR.elev, 1);

            newHRIRAvailable();
        } else {
            sleep(10);
        }
    }
}

bool HRIRLoader::submitJob(float azm, float elev) {
    if (hrirFinished.load()) {
        hrirFinished.store(false);

        requestedHRIR.azm = azm;
        requestedHRIR.elev = elev;

        jobSubmitted.store(true);

        return true;
    } else {
        return false;
    }
}

juce::AudioBuffer<float> &HRIRLoader::getHRIR() {
    return hrirBuffer;
}

void HRIRLoader::hrirAccessed() {
    hrirFinished.store(true);
}
