#ifndef VISION_H
#define VISION_H
#include <3ds.h>
#include "net.h"

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height);
void writePictureToMat(ncnn::Mat &mat, void *img, u16 x0, u16 y0, u16 width, u16 height);
void bordered_resize(ncnn::Mat &src, ncnn::Mat &dst, int w);

#endif // VISION_Hs