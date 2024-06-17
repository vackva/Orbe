#include "SofaReader.h"

SofaReader::~SofaReader() {
    mysofa_close(*sofa);
    sofa.reset();
}

void SofaReader::prepare(double samplerate)
{
    
    auto sofaBinary = BinaryData::pp2_HRIRs_measured_sofa;
    auto sofaSizeBinary = BinaryData::pp2_HRIRs_measured_sofaSize;
    
    switch (hrirChoice)
    {
        case hrirChoices::measured:
            sofaBinary = BinaryData::pp2_HRIRs_measured_sofa;
            sofaSizeBinary = BinaryData::pp2_HRIRs_measured_sofaSize;
            break;
        case hrirChoices::interpolated_sh:
            sofaBinary = BinaryData::pp2_HRIRs_interpolated_sh_sofa;
            sofaSizeBinary = BinaryData::pp2_HRIRs_interpolated_sh_sofaSize;
            break;
        case hrirChoices::interpolated_sh_timealign:
            sofaBinary = BinaryData::pp2_HRIRs_interpolated_sh_timealign_sofa;
            sofaSizeBinary = BinaryData::pp2_HRIRs_interpolated_sh_timealign_sofaSize;
            break;
        case hrirChoices::interpolated_mca:
            sofaBinary = BinaryData::pp2_HRIRs_interpolated_mca_time_aligned_sofa;
            sofaSizeBinary = BinaryData::pp2_HRIRs_interpolated_mca_time_aligned_sofaSize;
            break;
        default:
            break;
    }
    
    if (sofa != nullptr){
        sofa.reset();
    }
    int err;
    sofa = std::make_unique<MYSOFA_EASY*>(mysofa_open_data(sofaBinary, sofaSizeBinary, static_cast<float>(samplerate), &ir_length, &err));
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
