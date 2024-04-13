//
// Created by Valentin Ackva on 13/04/2024.
//

#ifndef BINAURALPANNER_SOFAREADER_H
#define BINAURALPANNER_SOFAREADER_H

#include <JuceHeader.h>
#include <saf.h>

class SofaReader {
public:
    SofaReader() {
        // SOFA_TEST_FILE_PATH defined in CMakeLists.txt - TODO load as Binary if possible
        error = saf_sofa_open(&sofa, SOFA_TEST_FILE_PATH, SAF_SOFA_READER_OPTION_DEFAULT);

        switch (error) {
            case SAF_SOFA_OK:
                std::cout << "Successfully loaded Sofa File" << std::endl;
                std::cout << "APIName: " << sofa.APIName << std::endl;
                std::cout << "APIVersion: " << sofa.APIVersion << std::endl;
                std::cout << "DataSamplingRate: " << sofa.DataSamplingRate << std::endl;
                break;
            case SAF_SOFA_ERROR_INVALID_FILE_OR_FILE_PATH:
                std::cout << "Not a SOFA file, or no such file was found in the specified location" << std::endl;
                break;
            case SAF_SOFA_ERROR_DIMENSIONS_UNEXPECTED:
                std::cout << "Dimensions of the SOFA data were not as expected" << std::endl;
                break;
            case SAF_SOFA_ERROR_FORMAT_UNEXPECTED:
                std::cout << "The data-type of the SOFA data was not as expected" << std::endl;
                break;
            case SAF_SOFA_ERROR_NETCDF_IN_USE:
                std::cout << "NetCDF is not thread safe!" << std::endl;
                break;
        }


    }

private:
    SAF_SOFA_ERROR_CODES error;
    saf_sofa_container sofa;
};

#endif //BINAURALPANNER_SOFAREADER_H
