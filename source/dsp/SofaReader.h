//
// Created by Valentin Ackva on 13/04/2024.
//

#ifndef BINAURALPANNER_SOFAREADER_H
#define BINAURALPANNER_SOFAREADER_H

#include <JuceHeader.h>
#include <mysofa.h>

enum sofaChoices
{
    measured,
    interpolated_sh,
    interpolated_sh_timealign,
    interpolated_mca
};

class SofaReader {
public:
    SofaReader() = default;
    ~SofaReader();

    void prepare(double samplerate);

    int get_ir_length( sofaChoices sofaChoice) ;
    void get_hrirs(juce::AudioBuffer<float>& buffer, float azim, float elev, float dist, float &currentLeftDelay, float &currentRightDelay, sofaChoices sofaChoice);

private:
    float coordinate_buffer[3];
    int ir_length_measured;
    int ir_length_interpolated_sh;
    int ir_length_interpolated_sh_timealign;
    int ir_length_interpolated_mca;
    //std::map<juce::String interpolation, std::unique_ptr<MYSOFA_EASY*> sofa_files;
    std::unique_ptr<MYSOFA_EASY*> sofa_measured;
    std::unique_ptr<MYSOFA_EASY*> sofa_interpolated_sh;
    std::unique_ptr<MYSOFA_EASY*> sofa_interpolated_sh_timealign;
    std::unique_ptr<MYSOFA_EASY*> sofa_interpolated_mca;
};

#endif //BINAURALPANNER_SOFAREADER_H
