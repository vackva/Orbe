//
// Created by Valentin Ackva on 13/04/2024.
//

#ifndef BINAURALPANNER_SOFAREADER_H
#define BINAURALPANNER_SOFAREADER_H

#include <JuceHeader.h>
#include <mysofa.h>

class SofaReader {
public:
    SofaReader() {
        // SOFA_TEST_FILE_PATH defined in CMakeLists.txt - TODO load as Binary if possible    
    }

    ~SofaReader() {
        mysofa_close(*sofa);
    }

    void prepare(int samplerate){
        if (sofa != nullptr){
            sofa.reset();
        }
        int err;
        std::cout << SOFA_TEST_FILE_PATH << std::endl;
        sofa = std::make_unique<MYSOFA_EASY*>(mysofa_open(SOFA_TEST_FILE_PATH, static_cast<float>(samplerate), &ir_length, &err));
        switch (err) {
            case MYSOFA_OK:
                std::cout << "Successfully loaded Sofa File" << std::endl;
                std::cout << "Length of IRs: " << ir_length << std::endl;
                break;
            default:
                std::cout << "Error while loading Sofa File" << std::endl;
                return;
        }
    }
    int get_ir_length(){
        return ir_length;
    }
    void get_hrirs(juce::AudioBuffer<float>* buffer, float azim, float elev, float dist){
        float* leftir = buffer->getWritePointer(0);
        float* rightir = buffer->getWritePointer(1);
        float leftdelay;
        float rightdelay; 
        // convert coordinates to xyz
        coordinate_buffer[0] = azim;
        coordinate_buffer[1] = elev;
        coordinate_buffer[2] = dist;
        mysofa_s2c((float *) &coordinate_buffer);

        mysofa_getfilter_float(*sofa, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftir, rightir, &leftdelay, &rightdelay);
    }

private:
    int ir_length;
    float coordinate_buffer[3];
    std::unique_ptr<MYSOFA_EASY*> sofa;
};

#endif //BINAURALPANNER_SOFAREADER_H
