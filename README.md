## Build instruction

Dependencies on Linux (macOS and Windows do not need this!)
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
