#ifndef NANODET_H
#define NANODET_H

#include "detector.h"
#include "net.h"
#include <iostream>


static void generate_grid_center_priors(const int input_height, const int input_width, std::vector<int>& strides, std::vector<CenterPrior>& center_priors);


class Nanodet : public Detector
{
    virtual BoxInfo disPred2Bbox(const float *&dfl_det, int label, float score, int x, int y, int stride);
    virtual void decode(ncnn::Mat &output, std::vector<CenterPrior> &center_priors, float threshold, std::vector<std::vector<BoxInfo>> &results);
    
    public:
        virtual void load_param(const char* json_file);
        virtual std::vector<BoxInfo> detect(ncnn::Mat &input);
};
#endif // NANODET_H