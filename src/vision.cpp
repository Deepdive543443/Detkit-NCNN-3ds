#include "vision.h"

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height)
{
	u8 *fb_8 = (u8*) fb;
	u16 *img_16 = (u16*) img;
	int i, j, draw_x, draw_y;
	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
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
    u16 *img_16 = (u16*) img;
    int cstep = mat.cstep;
    float *mat_ptr;

    

    // printf("w: %d, h: %d, c: %d, cstep: %d\n", mat.w, mat.h, mat.c, mat.cstep);
    // printf("Element size: %d\n", mat.elemsize);
    for (u16 j = 0; j < height; j++)
    {
        for (u16 i = 0; i < width; i++)
        {
            // if (i > 3 && j > 3) continue;

            mat_ptr = mat.row(j);
            // Read and decompress the data from buffer
            int draw_y = height - j;
			int draw_x = i;
			// u32 v = (draw_y + draw_x * height) * 3;
			u16 data = img_16[j * width + i];

            uint8_t b = ((data >> 11) & 0x1F) << 3;
			uint8_t g = ((data >> 5) & 0x3F) << 2;
			uint8_t r = (data & 0x1F) << 3;

            // write data in float type
            mat_ptr[i] =  (float) r; // R
            mat_ptr[cstep + i] =  (float) g; // G
            mat_ptr[cstep * 2 + i] = (float) b; // B

            // printf("int: %d, %d, %d\n", b, g, r);
            // printf("float: %f, %f, %f\n", (float) b, (float) g, (float) r);
        }
    }
}