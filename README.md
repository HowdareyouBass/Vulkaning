# Vulkaning

Some projects that i wanted to implement fully from scratch with vulkan hpp and sdl3. Those projects include:
1. Slime simulation
![Slimey](https://github.com/HowdareyouBass/Vulkaning/assets/62214754/122f75d4-00ae-4310-87d2-92d8d0075826)
2. Water
![water_rendering](https://github.com/HowdareyouBass/Vulkaning/assets/62214754/8f1e136a-78b1-47a4-8010-b17c785d8f85)


## How to build:

### Windows and Visual Studio
1. Clone or download the project sources
```shell
git clone --recurse-submodules https://github.com/HowdareyouBass/Vulkaning.git
```
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
To build the project on linux you need to install CMake and C++ 20 compiler such as gcc or clang. To clone and build the project run the following commands:
```shell
git clone --recurse-submodules https://github.com/HowdareyouBass/Vulkaning.git
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
