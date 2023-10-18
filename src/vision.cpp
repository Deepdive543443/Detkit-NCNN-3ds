#include "vision.h"

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height)
{
    u8 *fb_8 = (u8*) fb;
    u16 *img_16 = (u16*) img;
    int i, j, draw_x, draw_y;
    for(j = 0; j < height; j++) 
    {
        for(i = 0; i < width; i++) 
        {
            draw_y = y + height - j;
            draw_x = x + i;
            u32 v = (draw_y + draw_x * height) * 3;
            u16 data = img_16[j * width + i];
            uint8_t b = ((data >> 11) & 0x1F) << 3;
            uint8_t g = ((data >> 5) & 0x3F) << 2;
            uint8_t r = (data & 0x1F) << 3;
            fb_8[v] = r;
            fb_8[v+1] = g;
            fb_8[v+2] = b;
        }
    }
}

void writePictureToMat(ncnn::Mat &mat, void *img, u16 width, u16 height)
{
    u16 *img_16 = (u16 *) img;
    float *mat_ptr;
    u16 data;
    int cstep = mat.cstep;

    for (int j = 0; j < height; j++)
    {
        mat_ptr = mat.row(j);
        for (int i = 0; i < width; i++)
        {
            data = img_16[j * width + i];
            
            mat_ptr[0] = (float) ((data & 0x1F) << 3);
            mat_ptr[cstep] = (float) (((data >> 5) & 0x3F) << 2);
            mat_ptr[cstep * 2] = (float) (((data >> 11) & 0x1F) << 3);

            mat_ptr++;            
        }
    }
}