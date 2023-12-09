# Detkit-NCNN-3DS
![IMG_20231020_145410](https://github.com/Deepdive543443/Nanodet-plus-3ds/assets/83911295/259dfb26-80d7-4e4e-903b-79fdfccb83e3)
This project aims to make more lightweight object detection models run on Nintendo 3DS, powered by NCNN. 

This project has been renamed from Nanodet Plus 3DS to Detkit NCNN 3DS because I'm trying to add more light-weight object detection models into this app.
You can still find older version in [Release](https://github.com/Deepdive543443/Detkit-NCNN-3ds/releases) page.


## Update 
This project now includes:
- Nanodet-Plus(int8)
- Fastest Det

Thanks for the [3DS-cmake](https://github.com/Lectem/3ds-cmake), you can now use CMake to build this project. Or use the toolchain files to build your libraries for 3DS. 




## Download
Required Homebrew launcher or a title manager like FBI to install or boot.
[3dsx and CIA](https://github.com/Deepdive543443/Nanodet-plus-3ds/releases/tag/1.0.2)

[Pre-build NCNN lib for 3DS](https://github.com/Deepdive543443/Benchncnn-3DS/releases/tag/v0.0.0)

## Build
This project has dependencies on NCNN and RapidJSON.
Follow the build guide from [last project](https://github.com/Deepdive543443/Benchncnn-3DS/tree/main) to install the 3DS development toolchain.
Then build this project using Makefile
```
make -j4
```
Or CMake (Recommended if you want to link your library)
```
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/DevkitArm3DS.cmake ..
make -j4
```


## Issues
One core and 64mb RAM are too little for real-time detection


## Credit
- [NCNN](https://github.com/Tencent/ncnn): High performance neural network inference computing framework for mobile platform, easy to use and port
- [Nanodet-Plus](https://github.com/RangiLyu/nanodet):  Super light weight anchor-free object detection model
- [Fastest-Det](https://github.com/dog-qiuqiu/FastestDet): A FASTER, STRONGER, SIMPLER single scale anchor-free object detection model
- [RapidJSON](https://rapidjson.org/): Light-weight Header only JSON parser for C++ 11 and above.
- [DevkitPRO](https://devkitpro.org/wiki/Getting_Started): Toolchain for 3DS homebrew development
- [3DS-cmake](https://github.com/Xtansia/3ds-cmake): Toolchain files to build CMake project for 3DS
- [FTPD-Pro](https://github.com/mtheall/ftpd): FTP Server for 3DS/Switch/Linux.
- [Citra](https://github.com/citra-emu/citra): 3DS emulator for Windows, Linux, and MacOS
