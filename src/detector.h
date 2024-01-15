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

        Detector();
        virtual ~Detector();

        ncnn::Net detector;
        int input_size[2];
        float mean_vals[3];
        float norm_vals[3];

        void inference_test();
        int create(const char* param, const char* bin, const ncnn::Option &opt);
        void clear();
        void nms(std::vector<BoxInfo>& input_boxes, float NMS_THRESH);

        virtual void load_param(const char* json_file);
        virtual std::vector<BoxInfo> detect(cv::Mat &ocv_input);
        virtual void draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes);


};

#endif // DETECTOR_H