#ifndef NANODET_H
#define NANODET_H

#include "net.h"
// #include "simpleocv.h"
#include <vector>

typedef struct HeadInfo
{
    std::string cls_layer;
    std::string dis_layer;
    int stride;
};

struct CenterPrior
{
    int x;
    int y;
    int stride;
};

typedef struct BoxInfo
{
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
} BoxInfo;

inline float fast_exp(float x);
inline float sigmoid(float x);
// int activation_function_softmax(const _Tp* src, _Tp* dst, int length);
int activation_function_softmax(const float *src, float *dst, int length);
static void generate_grid_center_priors(const int input_height, const int input_width, std::vector<int>& strides, std::vector<CenterPrior>& center_priors);

class Nanodet
{
    ncnn::Net nanodet;
    int input_size[2] = {320, 320}; // input height and width
    int num_class = 80; // number of classes. 80 for COCO
    int reg_max = 7;
    std::vector<int> strides = { 8, 16, 32, 64};

    const float mean_vals[3] = { 103.53f, 116.28f, 123.675f };
    const float norm_vals[3] = { 0.017429f, 0.017507f, 0.017125f };

    std::vector<CenterPrior> center_priors;
    // std::vector<std::vector<BoxInfo>> results;


    virtual void decode(ncnn::Mat& feats, std::vector<CenterPrior>& center_priors, float threshold, std::vector<std::vector<BoxInfo>>& results);
    // generate_grid_center_priors(this->input_size[0], this->input_size[1], this->strides, center_priors);
    virtual BoxInfo disPred2Bbox(const float *&dfl_det, int label, float score, int x, int y, int stride);
    virtual void nms(std::vector<BoxInfo>& input_boxes, float NMS_THRESH);
    // ncnn::Extractor extractor;
    // std::vector<char*>& input_names;
    // std::vector<char*>& output_names;

    public:
        // Nanodet(const char* param, const char* bin, const ncnn::Option &opt);
        virtual int create(const char* param, const char* bin, const ncnn::Option &opt);
        virtual void inference_test(ncnn::Mat &input);
        virtual std::vector<BoxInfo> detect(ncnn::Mat &input);
};

#endif // NANODET_H