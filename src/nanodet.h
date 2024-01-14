#ifndef NANODET_H
#define NANODET_H

#include "detector.h"
#include "net.h"
#include "vision.h"
#include <iostream>
#include <cfloat>


static void generate_grid_center_priors(const int input_height, const int input_width, std::vector<int>& strides, std::vector<CenterPrior>& center_priors);
class Nanodet : public Detector
{
    BoxInfo disPred2Bbox(const float *&dfl_det, int label, float score, int x, int y, int stride);
    void decode(ncnn::Mat &output, std::vector<CenterPrior> &center_priors, float threshold, std::vector<std::vector<BoxInfo>> &results);
    
    public:
        void load_param(const char* json_file);
        std::vector<BoxInfo> detect(cv::Mat &ocv_input, float prob_threshold = 0.4f, float nms_threshold = 0.5f);
        void draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes);
};
#endif // NANODET_H