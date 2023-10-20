# Nanodet-plus-3ds
![IMG_20231020_145410](https://github.com/Deepdive543443/Nanodet-plus-3ds/assets/83911295/29fbc40e-2468-4afd-ad26-4b238770eb0a)



## Download
Required Homebrew launcher or a title manager like FBI to install or boot.
[3dsx and CIA](https://github.com/Deepdive543443/Nanodet-plus-3ds/releases/tag/Nanodet-plus-3DS)

## Build
Read the build guide from [last project](https://github.com/Deepdive543443/Benchncnn-3DS/tree/main) 


## Issues
One core and 64mb is too little for real-time detection
This demo is using 416x416 weight for 320x320 input because I haven't find 320x320 weights for NCNN available. Also, INT8 performance is not tested


## Credit
- [NCNN](https://github.com/Tencent/ncnn): High performance neural network inference computing framework for mobile platform, easy to use and port
- [Nanodet-Plus](https://github.com/RangiLyu/nanodet):  Super light weight anchor-free object detection model
- [DevkitPRO](https://devkitpro.org/wiki/Getting_Started): Toolchain for 3DS homebrew development
- [3DS-cmake](https://github.com/Xtansia/3ds-cmake): Toolchain files to build CMake project for 3DS
- [FTPD-Pro](https://github.com/mtheall/ftpd): FTP Server for 3DS/Switch/Linux.
- [Citra](https://github.com/citra-emu/citra): 3DS emulator for Windows, Linux, and MacOS
