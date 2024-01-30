# Vulkaning

Vulkan engine written by me.

## How to build:

### Windows and Visual Studio
1. Clone or download the project sources
2. Download CMake
3. Run the following commands from command prompt
```shell
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release .. 
```
This will generate Visual Studio solution file.
Didn't test it though.

### Linux
To build the project on linux you need to install CMake and C++ 20 compiler such as gcc or clang. Run the following commands from the project root.
```shell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### To run the project:
```shell
cd ..
./build/src/vulkaning
```
