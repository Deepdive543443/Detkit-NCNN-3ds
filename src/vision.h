#ifndef VISION_H
#define VISION_H
#include <3ds.h>
#include "net.h"

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height);
void writePictureToMat(ncnn::Mat &mat, void *img, u16 width, u16 height);

#endif // VISION_Hs