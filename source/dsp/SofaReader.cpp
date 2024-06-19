#include "SofaReader.h"

SofaReader::~SofaReader() {
    
    //MEASURED
    mysofa_close(*sofa_measured);
    sofa_measured.reset();
    
    // INTERPOLATION SH
    mysofa_close(*sofa_interpolated_sh);
    sofa_interpolated_sh.reset();
    
    // INTERPOLATION SH TIMEALIGNED
    mysofa_close(*sofa_interpolated_sh_timealign);
    sofa_interpolated_sh_timealign.reset();
    
    // INTERPOLATION MCA
    mysofa_close(*sofa_interpolated_mca);
    sofa_interpolated_mca.reset();
}

void SofaReader::prepare(double samplerate)
{
    // MEASURED
    auto sofaBinary = BinaryData::pp2_HRIRs_measured_sofa;
    auto sofaSizeBinary = BinaryData::pp2_HRIRs_measured_sofaSize;
    
    if (sofa_measured != nullptr)
    {
        sofa_measured.reset();
    }
    
    int err;
    sofa_measured = std::make_unique<MYSOFA_EASY*>(mysofa_open_data(sofaBinary, sofaSizeBinary, static_cast<float>(samplerate), &ir_length_measured, &err));
    switch (err) 
    {
        case MYSOFA_OK:
            std::cout << "Successfully loaded Sofa File" << std::endl;
            std::cout << "Length of IRs: " << ir_length_measured << std::endl;
            break;
        default:
            std::cout << "Error while loading Sofa File" << std::endl;
            return;
    }
            
    // INTERPOLATED SH
    sofaBinary = BinaryData::pp2_HRIRs_interpolated_sh_sofa;
    sofaSizeBinary = BinaryData::pp2_HRIRs_interpolated_sh_sofaSize;
    
    if (sofa_interpolated_sh != nullptr)
    {
        sofa_interpolated_sh.reset();
    }
    
    err;
    sofa_interpolated_sh = std::make_unique<MYSOFA_EASY*>(mysofa_open_data(sofaBinary, sofaSizeBinary, static_cast<float>(samplerate), &ir_length_interpolated_sh, &err));
    switch (err) 
    {
        case MYSOFA_OK:
            std::cout << "Successfully loaded Sofa File" << std::endl;
            std::cout << "Length of IRs: " << ir_length_interpolated_sh << std::endl;
            break;
        default:
            std::cout << "Error while loading Sofa File" << std::endl;
            return;
    }
            
    // INTERPOLATED SH TIMEALIGNED
    sofaBinary = BinaryData::pp2_HRIRs_interpolated_sh_timealign_sofa;
    sofaSizeBinary = BinaryData::pp2_HRIRs_interpolated_sh_timealign_sofaSize;
    
    if (sofa_interpolated_sh_timealign != nullptr)
    {
        sofa_interpolated_sh_timealign.reset();
    }
    
    err;
    sofa_interpolated_sh_timealign = std::make_unique<MYSOFA_EASY*>(mysofa_open_data(sofaBinary, sofaSizeBinary, static_cast<float>(samplerate), &ir_length_interpolated_sh_timealign, &err));
    switch (err) 
    {
        case MYSOFA_OK:
            std::cout << "Successfully loaded Sofa File" << std::endl;
            std::cout << "Length of IRs: " << ir_length_interpolated_sh_timealign << std::endl;
            break;
        default:
            std::cout << "Error while loading Sofa File" << std::endl;
            return;
    }
            
    // INTERPOLATED SH TIMEALIGNED
    sofaBinary = BinaryData::pp2_HRIRs_interpolated_mca_time_aligned_sofa;
    sofaSizeBinary = BinaryData::pp2_HRIRs_interpolated_mca_time_aligned_sofaSize;
    
    if (sofa_interpolated_mca != nullptr)
    {
        sofa_interpolated_mca.reset();
    }
    
    err;
    sofa_interpolated_mca = std::make_unique<MYSOFA_EASY*>(mysofa_open_data(sofaBinary, sofaSizeBinary, static_cast<float>(samplerate), &ir_length_interpolated_mca, &err));
    switch (err) 
    {
        case MYSOFA_OK:
            std::cout << "Successfully loaded Sofa File" << std::endl;
            std::cout << "Length of IRs: " << ir_length_interpolated_mca << std::endl;
            break;
        default:
            std::cout << "Error while loading Sofa File" << std::endl;
            return;
    }
}

int SofaReader::get_ir_length( sofaChoices sofaChoice ) {
    switch(sofaChoice)
    {
        case sofaChoices::measured:
            return ir_length_measured;
            
        case sofaChoices::interpolated_sh:
            return ir_length_interpolated_sh;
            
        case sofaChoices::interpolated_sh_timealign:
            return ir_length_interpolated_sh_timealign;
            
        case sofaChoices::interpolated_mca:
            return ir_length_interpolated_mca;
            
        default:
            return ir_length_measured;
    }
}

void SofaReader::get_hrirs(AudioBuffer<float> &buffer, float azim, float elev, float dist, float &leftDelay, float &rightDelay, sofaChoices sofaChoice, bool doNearestNeighbourInterpolation) {
    auto leftIR = buffer.getWritePointer(0);
    auto rightIR = buffer.getWritePointer(1);
    //float leftDelay;
    //float rightDelay;
    // convert coordinates to xyz
    coordinate_buffer[0] = azim;
    coordinate_buffer[1] = elev;
    coordinate_buffer[2] = dist;
    mysofa_s2c((float *) &coordinate_buffer);
    
    switch(sofaChoice)
    {
        case sofaChoices::measured:
            if ( doNearestNeighbourInterpolation )
                mysofa_getfilter_float(*sofa_measured, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            else
                mysofa_getfilter_float_nointerp(*sofa_measured, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            break;
            
        case sofaChoices::interpolated_sh:
            if ( doNearestNeighbourInterpolation )
                mysofa_getfilter_float(*sofa_interpolated_sh, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            else
                mysofa_getfilter_float_nointerp(*sofa_interpolated_sh, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            break;
            
        case sofaChoices::interpolated_sh_timealign:
            if ( doNearestNeighbourInterpolation )
                mysofa_getfilter_float(*sofa_interpolated_sh_timealign, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            else
                mysofa_getfilter_float_nointerp(*sofa_interpolated_sh_timealign, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            break;
            
        case sofaChoices::interpolated_mca:
            if ( doNearestNeighbourInterpolation )
                mysofa_getfilter_float(*sofa_interpolated_mca, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            else
                mysofa_getfilter_float_nointerp(*sofa_interpolated_mca, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            break;
            
        default:
            if ( doNearestNeighbourInterpolation )
                mysofa_getfilter_float(*sofa_measured, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            else
                mysofa_getfilter_float_nointerp(*sofa_interpolated_mca, coordinate_buffer[0], coordinate_buffer[1], coordinate_buffer[2], leftIR, rightIR, &leftDelay, &rightDelay);
            break;
    }
}
