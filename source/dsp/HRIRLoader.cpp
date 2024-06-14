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
            
            // set previous hrir to last temp hrir
            //previousHrirBuffer.makeCopyOf(tempHrirBuffer);
            
            // get current hrir
            currentHrirBuffer.setSize(currentSpec.numChannels, sofaReader.get_ir_length());
            sofaReader.get_hrirs(currentHrirBuffer, requestedHRIR.azm, requestedHRIR.elev, 1, currentLeftDelay, currentRightDelay);
            
            // copy current hrir to temp hrir
            //tempHrirBuffer.makeCopyOf(currentHrirBuffer);

            newHRIRAvailable();
        } else {
            sleep(10);
        }
    }
}

void HRIRLoader::getCurrentDelays(float &left, float &right, float samplerate){
    left = currentLeftDelay / 44100 * samplerate;
    right = currentRightDelay / 44100 * samplerate;
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

juce::AudioBuffer<float> &HRIRLoader::getCurrentHRIR() {
    return currentHrirBuffer;
}

/*juce::AudioBuffer<float> &HRIRLoader::getPreviousHRIR() {
    return previousHrirBuffer;
}*/

void HRIRLoader::hrirAccessed() {
    hrirFinished.store(true);
}
