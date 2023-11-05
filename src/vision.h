#ifndef VISION_H
#define VISION_H
#include <3ds.h>
#include "net.h"
#include "nanodet.h"
#include "simpleocv.h"

struct object_rect {
    int x;
    int y;
    int width;
    int height;
};

const int color_list[80][3] =
{
    //{255 ,255 ,255}, //bg
    {216 , 82 , 24},
    {236 ,176 , 31},
    {125 , 46 ,141},
    {118 ,171 , 47},
    { 76 ,189 ,237},
    {238 , 19 , 46},
    { 76 , 76 , 76},
    {153 ,153 ,153},
    {255 ,  0 ,  0},
    {255 ,127 ,  0},
    {190 ,190 ,  0},
    {  0 ,255 ,  0},
    {  0 ,  0 ,255},
    {170 ,  0 ,255},
    { 84 , 84 ,  0},
    { 84 ,170 ,  0},
    { 84 ,255 ,  0},
    {170 , 84 ,  0},
    {170 ,170 ,  0},
    {170 ,255 ,  0},
    {255 , 84 ,  0},
    {255 ,170 ,  0},
    {255 ,255 ,  0},
    {  0 , 84 ,127},
    {  0 ,170 ,127},
    {  0 ,255 ,127},
    { 84 ,  0 ,127},
    { 84 , 84 ,127},
    { 84 ,170 ,127},
    { 84 ,255 ,127},
    {170 ,  0 ,127},
    {170 , 84 ,127},
    {170 ,170 ,127},
    {170 ,255 ,127},
    {255 ,  0 ,127},
    {255 , 84 ,127},
    {255 ,170 ,127},
    {255 ,255 ,127},
    {  0 , 84 ,255},
    {  0 ,170 ,255},
    {  0 ,255 ,255},
    { 84 ,  0 ,255},
    { 84 , 84 ,255},
    { 84 ,170 ,255},
    { 84 ,255 ,255},
    {170 ,  0 ,255},
    {170 , 84 ,255},
    {170 ,170 ,255},
    {170 ,255 ,255},
    {255 ,  0 ,255},
    {255 , 84 ,255},
    {255 ,170 ,255},
    { 42 ,  0 ,  0},
    { 84 ,  0 ,  0},
    {127 ,  0 ,  0},
    {170 ,  0 ,  0},
    {212 ,  0 ,  0},
    {255 ,  0 ,  0},
    {  0 , 42 ,  0},
    {  0 , 84 ,  0},
    {  0 ,127 ,  0},
    {  0 ,170 ,  0},
    {  0 ,212 ,  0},
    {  0 ,255 ,  0},
    {  0 ,  0 , 42},
    {  0 ,  0 , 84},
    {  0 ,  0 ,127},
    {  0 ,  0 ,170},
    {  0 ,  0 ,212},
    {  0 ,  0 ,255},
    {  0 ,  0 ,  0},
    { 36 , 36 , 36},
    { 72 , 72 , 72},
    {109 ,109 ,109},
    {145 ,145 ,145},
    {182 ,182 ,182},
    {218 ,218 ,218},
    {  0 ,113 ,188},
    { 80 ,182 ,188},
    {127 ,127 ,  0},
};

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

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height);
void writePictureToMat(ncnn::Mat &mat, void *img, u16 x0, u16 y0, u16 width, u16 height);
void writeMatToFrameBuf(ncnn::Mat &mat, void *buf, u16 x, u16 y, u16 width, u16 height);
void writeMatToFrameBuf(cv::Mat &mat, void *buf, u16 x, u16 y, u16 width, u16 height);
void bordered_resize(ncnn::Mat &src, ncnn::Mat &dst, int dst_w, int draw_coor);
double get_current_time();
void draw_bboxes(const cv::Mat &bgr, const std::vector<BoxInfo> &bboxes, int v_shift, float scaler);
#endif // VISION_H