#ifndef DETECTOR_H
#define DETECTOR_H

#include "net.h"
#include "simpleocv.h"
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

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
float fast_sigmoid(float x);
float fast_tanh(float x);
int activation_function_softmax(const float *src, float *dst, int length);

class Detector
{
   public:
        ncnn::Net detector;
        int input_size[2];
        float mean_vals[3];
        float norm_vals[3];        

        int create(const char* param, const char* bin, const ncnn::Option &opt);
        void inference_test();
        void nms(std::vector<BoxInfo>& input_boxes, float NMS_THRESH);
};

#endif // DETECTOR_H