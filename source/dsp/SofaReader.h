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
                return;
            case SAF_SOFA_ERROR_DIMENSIONS_UNEXPECTED:
                std::cout << "Dimensions of the SOFA data were not as expected" << std::endl;
                return;
            case SAF_SOFA_ERROR_FORMAT_UNEXPECTED:
                std::cout << "The data-type of the SOFA data was not as expected" << std::endl;
                return;
            case SAF_SOFA_ERROR_NETCDF_IN_USE:
                std::cout << "NetCDF is not thread safe!" << std::endl;
                return;
        }

        //
        int azimuthStep = 30;
        int elevationStep = 30;
        int minElevation = -30;
        int numElevationSteps = 5;

        int azimuth1 = 90, elevation1 = 45;

        // Calculate source indices
        int indexTest = calculateSourceIndex(azimuth1, elevation1, azimuthStep, elevationStep, minElevation, numElevationSteps);

        // Print results
        printf("Source index for azimuth %d, elevation %d: %d\n", azimuth1, elevation1, indexTest);

        int leftEarIndex = 0;
        int rightEarIndex = 1;

        auto hrtfLeft = accessHRTF(&sofa, indexTest, leftEarIndex);
        auto hrtfRight = accessHRTF(&sofa, indexTest, rightEarIndex);

        std::cout << "Successfully loaded HRTF" << std::endl;

        writeHRTFToWav(hrtfLeft, hrtfRight, "HRTF_Output.wav");
    }

    // Function to calculate the source index for a given azimuth and elevation
    int calculateSourceIndex(int azimuth, int elevation, int azimuthStep, int elevationStep, int minElevation, int numElevationSteps) {
        int azimuthIndex = azimuth / azimuthStep;
        int elevationIndex = (elevation - minElevation) / elevationStep;
        return azimuthIndex * numElevationSteps + elevationIndex;
    }

    int getSourceReceiverIndex(int sourceIndex, int receiverIndex, saf_sofa_container *sofaToUse) {
        return (sourceIndex * sofaToUse->nReceivers + receiverIndex) * sofaToUse->DataLengthIR;
    }

    // not real-time safe!!
    std::vector<float> accessHRTF(saf_sofa_container *sofaToUse, int sourceIndex, int receiverIndex) {
        int index = getSourceReceiverIndex(sourceIndex, receiverIndex, sofaToUse);
        std::vector<float> hrtf(sofaToUse->DataLengthIR);
        memcpy(hrtf.data(), &sofaToUse->DataIR[index], sizeof(float) * sofaToUse->DataLengthIR);
        return hrtf;
    }

    void writeHRTFToWav(const std::vector<float>& hrtfLeft, const std::vector<float>& hrtfRight, const String& fileName) {
        File file(File::getCurrentWorkingDirectory().getChildFile(fileName));
        file.deleteFile();

        std::unique_ptr<FileOutputStream> fileStream(file.createOutputStream());
        if (!fileStream) {
            std::cout << "Failed to create file stream!" << std::endl;
            return;
        }

        WavAudioFormat wavFormat;
        std::unique_ptr<AudioFormatWriter> writer;
        writer.reset(wavFormat.createWriterFor(fileStream.get(), sofa.DataSamplingRate,
                                               2, 16, {}, 0));
        if (writer == nullptr) {
            std::cout << "Failed to create audio writer!" << std::endl;
            return;
        }

        juce::AudioBuffer<float> buffer(2, (int) hrtfLeft.size());
        buffer.copyFrom(0, 0, hrtfLeft.data(), (int) hrtfLeft.size());
        buffer.copyFrom(1, 0, hrtfRight.data(), (int) hrtfRight.size());

        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        fileStream->flush();

        std::cout << "HRTF data written to " << file.getFullPathName() << std::endl;

        fileStream.release();
    }

private:
    SAF_SOFA_ERROR_CODES error;
    saf_sofa_container sofa;
};

#endif //BINAURALPANNER_SOFAREADER_H
