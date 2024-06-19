# Orbe
![interface](assets/images/orbe-github-header.png)

## Overview
Orbe is a real-time audio plugin designed as a binaural spatializer

**Features:**
- **Realistic Binaural Panning:** Experience true-to-life binaural sound panning in an open field using sophisticated SOFA data handling
- **Flexible Source Positioning:** Control the audio source with precision using both Cartesian and spherical coordinates
- **3D Auto-Pan Functionality:** Simplify spatial manipulation with an auto-pan feature controlled by sinusoidal oscillators
- **3D Visualization:** Orbe provides a comprehensive visual representation of the sound field, enhancing the understanding of spatial dynamics

## Build instruction
Build with CMake
```bash
# clone the repository
git clone https://github.com/vackva/Orbe/
cd Orbe/

# initialize and set up submodules
git submodule update --init --recursive

# use cmake to build debug
cmake . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --config Debug

# use cmake to build release
cmake . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

## License

The primary license for the code of this project is the MIT license, but be aware of the licenses of the submodules:

 - The *JUCE* library is licensed under the [JUCE License](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md)
 - The *libmysofa* library is licensed under the [Custom License](https://github.com/hoene/libmysofa/blob/main/LICENSE)
 - The HRTF used in this project belong to the *HUTUBS* dataset and is licensed under the [CC BY 4.0 Deed](https://depositonce.tu-berlin.de/items/dc2a3076-a291-417e-97f0-7697e332c960)
