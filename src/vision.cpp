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

void writePictureToMat(ncnn::Mat &mat, void *img, u16 x0, u16 y0, u16 width, u16 height)
{
    u16 *img_16 = (u16 *) img;
    float *mat_ptr;
    u16 data;
    int cstep = mat.cstep;

    for (int j = y0; j < height; j++)
    {
        mat_ptr = mat.row(j);
        for (int i = x0; i < width; i++)
        {
            data = img_16[j * width + i];
            
            mat_ptr[0] = (float) ((data & 0x1F) << 3);
            mat_ptr[cstep] = (float) (((data >> 5) & 0x3F) << 2);
            mat_ptr[cstep * 2] = (float) (((data >> 11) & 0x1F) << 3);

            mat_ptr++;            
        }
    }
}

void writeMatToFrameBuf(ncnn::Mat &mat, void *buf, u16 x, u16 y, u16 width, u16 height)
{
    cv::Mat ocv_mat(mat.c, mat.h, mat.w);
    mat.to_pixels(ocv_mat.data, ncnn::Mat::PIXEL_BGR);
    writeMatToFrameBuf(ocv_mat, buf, x, y, width, height);
}

void writeMatToFrameBuf(cv::Mat &mat, void *buf, u16 x, u16 y, u16 width, u16 height)
{
    u8 *fb_8 = (u8 *) buf;
    u8 *image_ptr = (u8 *) mat.data;

    int draw_x, draw_y;
    for(int j = 0; j < height; j++) 
    {
        for(int i = 0; i < width; i++) 
        {
            draw_y = y + height - j;
            draw_x = x + i;
            u32 v = (draw_y + draw_x * height) * 3;

            fb_8[v] = image_ptr[0];
            fb_8[v+1] = image_ptr[1];
            fb_8[v+2] = image_ptr[2];

            image_ptr += 3;
        }
    }
}

void bordered_resize(ncnn::Mat &src, ncnn::Mat &dst, int dst_w, int draw_coor)
{
    int dst_h = src.h * ((float) dst_w / src.w);
    
    ncnn::Mat resized(dst_w, dst_h, 3);
    ncnn::resize_bilinear(
        src, 
        resized, 
        dst_w, 
        dst_h
    );

    float *dst_ptr = (float *) dst.data;
    float *resized_src_ptr = (float *) resized.data;

    int src_cstep = resized.cstep;
    int dst_cstep = dst.cstep;

    memset(dst_ptr, 0.f, dst_w * dst_w * 3);
    
    for (int c=0; c<3; c++)
    {
        dst_ptr = dst.row(draw_coor) + (c * dst_cstep);
        resized_src_ptr = resized.row(0) + (c * src_cstep);
        for (int j = 0; j < dst_h; j++)
        {
            for (int i = 0; i < dst_w; i++)
            {
                dst_ptr[0] = resized_src_ptr[0];
                dst_ptr++;
                resized_src_ptr++;
            }
        }
    }
}

void draw_bboxes(const cv::Mat& image, const std::vector<BoxInfo>& bboxes, int v_shift, float x_scaler, float y_scaler)
{
    const char line[] = "=======================================\n";
    fprintf(stderr, line);
    fprintf(stderr, "% 12s % 2s % ", "Label", "Score");
    fprintf(stderr, "% 4s % 4s % 4s % 4s\n\n","x1", "y1", "x2", "y2");

    for (size_t i = 0; i < bboxes.size(); i++)
    {
        const BoxInfo& bbox = bboxes[i];

        float x1 = bbox.x1;
        float y1 = bbox.y1;
        float w = (bbox.x2 - x1);
        float h = (bbox.y2 - y1);

        y1 -= v_shift;
        x1 *= x_scaler;
        y1 *= y_scaler;
        w *= x_scaler;
        h *= y_scaler;

        union
        {
            struct 
            {
                u8 r;
                u8 g;
                u8 b;
                u8 a;
            };
            u32 rgba;
        } color;

        color.r = color_list[bbox.label][0];
        color.g = color_list[bbox.label][1];
        color.b = color_list[bbox.label][2];
        color.a = 255;
        
        fprintf(stderr, "% 12s %02.3f % ", class_names[bbox.label], bbox.score);
        fprintf(stderr, "% 4d % 4d % 4d % 4d\n",(int) x1, (int) y1, (int) w, (int) h);

        ncnn::draw_rectangle_c3(
            image.data,
            image.cols,
            image.rows,
            (int) x1,
            (int) y1,
            (int) w,
            (int) h,
            (int) color.rgba, 
            2
        );

        ncnn::draw_text_c3(
            image.data,
            image.cols,
            image.rows, 
            class_names[bbox.label],
            (int) x1 + 1, 
            (int) y1 + 1,
            7,
            (int) color.rgba
        );
    }
    fprintf(stderr, line);
}