#include "SofaReader.h"

SofaReader::~SofaReader() {
    mysofa_close(*sofa);
    sofa.reset();
}

void SofaReader::prepare(double samplerate) {
    if (sofa != nullptr){
        sofa.reset();
    }
    int err;
    sofa = std::make_unique<MYSOFA_EASY*>(mysofa_open_data(BinaryData::pp2_HRIRs_measured_time_aligned_sofa, BinaryData::pp2_HRIRs_measured_time_aligned_sofaSize, static_cast<float>(44100), &ir_length, &err));
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

int SofaReader::get_ir_length() {
    return ir_length;
}

void SofaReader::get_hrirs(AudioBuffer<float> &buffer, float azim, float elev, float dist, float &leftDelay, float &rightDelay) {
    auto leftIR = buffer.getWritePointer(0);
    auto rightIR = buffer.getWritePointer(1);
    //float leftDelay;
    //float rightDelay;
    // convert coordinates to xyz
    coordinate_buffer[0] = azim;
    coordinate_buffer[1] = elev;
    coordinate_buffer[2] = dist;
    mysofa_s2c((float *) &coordinate_buffer);

    mysofa_getfilter_float(*sofa, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
}
