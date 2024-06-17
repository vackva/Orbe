//
// Created by Valentin Ackva on 13/04/2024.
//

#ifndef BINAURALPANNER_SOFAREADER_H
#define BINAURALPANNER_SOFAREADER_H

#include <JuceHeader.h>
#include <mysofa.h>

enum hrirChoices
{
    measured,
    interpolated_sh,
    interpolated_sh_timealign,
    interpolated_mca
};

//static hrirChoices hrirChoice = hrirChoices::measured;

class SofaReader {
public:
    SofaReader() = default;
    ~SofaReader();

    void prepare(double samplerate, hrirChoices hrirChoice);

    int get_ir_length();
    void get_hrirs(juce::AudioBuffer<float>& buffer, float azim, float elev, float dist, float &currentLeftDelay, float &currentRightDelay);

private:
    int ir_length;
    float coordinate_buffer[3];
    std::unique_ptr<MYSOFA_EASY*> sofa;
};

#endif //BINAURALPANNER_SOFAREADER_H
