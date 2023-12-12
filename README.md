# Detkit-NCNN-3DS
![IMG_20231020_145410](https://github.com/Deepdive543443/Nanodet-plus-3ds/assets/83911295/259dfb26-80d7-4e4e-903b-79fdfccb83e3)


## Intro
This project provides a 3DS Homebrew that enable Light-weight object detection on Nintendo 3DS.
It's still under development to integrate with more light-weight object detection networks, and better HCI experience


## Features 
This project now support none real-time object detection powered by:
- Nanodet-Plus(int8) (~11s)
- Fastest Det (~4s)

## Download and build
The Homebrew application from this project required a modded Nintendo 3DS with an title manager or Homebrew Launcher to boot.
You can download the pre-built application from Release: 
[3dsx and CIA](https://github.com/Deepdive543443/Nanodet-plus-3ds/releases/tag/1.0.2)

The Homebrew application has dependencies on libctru, ncnn, and RapidJSON. To build this Homebrew application by yourself, follow the build guide from [last project](https://github.com/Deepdive543443/Benchncnn-3DS/tree/main) to install the required toolchains and libraries and build this project by yourself.


Build this project using Makefile (Recommended)
```
make -j4
```
Or CMake
```
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/DevkitArm3DS.cmake ..
make -j4
```

Makefile is recommended because it has better support on other toolkit from devkitARM such as Makerom.

## Issues and limitation
Nintendo 3DS only has 64mb and single ARM11 core available for Homebrew and games to use, which brings some challanges on parallel computing and memory management.
CMake support was added but it's not compatible with some of the tools such as Makerom(CIA builder). We are still looking for proper solutions for issues above.


## Credit
- [NCNN](https://github.com/Tencent/ncnn): High performance neural network inference computing framework for mobile platform, easy to use and port
- [Nanodet-Plus](https://github.com/RangiLyu/nanodet):  Super light weight anchor-free object detection model
- [Fastest-Det](https://github.com/dog-qiuqiu/FastestDet): A FASTER, STRONGER, SIMPLER single scale anchor-free object detection model
- [RapidJSON](https://rapidjson.org/): Light-weight Header only JSON parser for C++ 11 and above.
- [DevkitPRO](https://devkitpro.org/wiki/Getting_Started): Toolchain for 3DS homebrew development
- [3DS-cmake](https://github.com/Xtansia/3ds-cmake): Toolchain files to build CMake project for 3DS
- [FTPD-Pro](https://github.com/mtheall/ftpd): FTP Server for 3DS/Switch/Linux.
- [Citra](https://github.com/citra-emu/citra): 3DS emulator for Windows, Linux, and MacOS
