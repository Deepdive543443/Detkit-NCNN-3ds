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

void bordered_resize(ncnn::Mat &src, ncnn::Mat &dst, int w)
{
    
    int h = 192;
    ncnn::Mat resized(320, 192, 3);
    ncnn::resize_bilinear(
        src, 
        resized, 
        320, 
        192
    );

    float *dst_ptr = (float *) dst.data;
    float *resized_src_ptr = (float *) resized.data;

    int src_cstep = resized.cstep;
    int dst_cstep = dst.cstep;

    memset(dst_ptr, 0.f, w * w * 3);
    for (int c=0; c<3; c++)
    {
        dst_ptr = dst.row(80) + (c * dst_cstep);
        resized_src_ptr = resized.row(0) + (c * src_cstep);
        for (int j = 0; j < h; j++)
        {
            for (int i = 0; i < w; i++)
            {
                dst_ptr[0] = resized_src_ptr[0];
                dst_ptr++;
                resized_src_ptr++;
            }
        }
    }

    // for (int j = 0; j < h; j++)
    // {
    //     dst_ptr = dst.row(j + first_row);
    //     resized_src_ptr = resized.row(j);
    //     for (int i = 0; i < w; i++)
    //     {
    //         for (int c = 0; c < 3; c++)
    //         {
    //             dst_ptr[c * dst_cstep] = resized_src_ptr[c * src_cstep];
    //         }   
    //     }
    //     dst_ptr++;
    //     resized_src_ptr++;
    // }

    // memcpy(dst_ptr, resized_src_ptr, w * h * 3);
}

void draw_bboxes(const cv::Mat& bgr, const std::vector<BoxInfo>& bboxes, object_rect effect_roi)
{
    static const char* class_names[] = { "person", "bicycle", "car", "motorcycle", "airplane", "bus",
                                        "train", "truck", "boat", "traffic light", "fire hydrant",
                                        "stop sign", "parking meter", "bench", "bird", "cat", "dog",
                                        "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
                                        "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
                                        "skis", "snowboard", "sports ball", "kite", "baseball bat",
                                        "baseball glove", "skateboard", "surfboard", "tennis racket",
                                        "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
                                        "banana", "apple", "sandwich", "orange", "broccoli", "carrot",
                                        "hot dog", "pizza", "donut", "cake", "chair", "couch",
                                        "potted plant", "bed", "dining table", "toilet", "tv", "laptop",
                                        "mouse", "remote", "keyboard", "cell phone", "microwave", "oven",
                                        "toaster", "sink", "refrigerator", "book", "clock", "vase",
                                        "scissors", "teddy bear", "hair drier", "toothbrush"
    };

    cv::Mat image = bgr.clone();
    int src_w = image.cols;
    int src_h = image.rows;
    int dst_w = effect_roi.width;
    int dst_h = effect_roi.height;
    float width_ratio = (float)src_w / (float)dst_w;
    float height_ratio = (float)src_h / (float)dst_h;


    for (size_t i = 0; i < bboxes.size(); i++)
    {
        const BoxInfo& bbox = bboxes[i];
        // printf("%f %f %f %f %d %f\n",bbox.x1, bbox.x2, bbox.y1, bbox.y2, bbox.label, bbox.score);
        printf("%s\n", class_names[bbox.label]);
    }

    // cv::imshow("image", image);
}