# Orbe
![interface](assets/images/orbe-github-header.png)

## Build instruction

Dependencies on Linux
```bash
# on Linux install
sudo apt-get update && sudo apt-get install libhdf5-dev libnetcdf-dev libnetcdff-dev liblapack3 liblapack-dev libopenblas-base libopenblas-dev liblapacke-dev
```

Build with CMake
```bash
# clone the repository
git clone https://github.com/vackva/SpatialPanner/
cd SpatialPanner/

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
 - The *Spatial_Audio_Framework* library is licensed under the [ISC License](https://github.com/leomccormack/Spatial_Audio_Framework/blob/master/LICENSE.md)
 - The *OpenBlas* library (Windows & Linux only) is licensed under the [BSD3](https://github.com/OpenMathLib/OpenBLAS/blob/develop/LICENSE)
 - The HRTF used in this project belong to the *HUTUBS* dataset and is licensed under the [CC BY 4.0 Deed](https://depositonce.tu-berlin.de/items/dc2a3076-a291-417e-97f0-7697e332c960)
