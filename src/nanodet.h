#ifndef NANODET_H
#define NANODET_H

#include "detector.h"
#include "net.h"
#include "vision.h"
#include <iostream>
#include <cfloat>

class Nanodet : public Detector
{
    
    public:
        virtual void load_param(const char* json_file);
        virtual std::vector<BoxInfo> detect(cv::Mat &ocv_input, float prob_threshold = 0.4f, float nms_threshold = 0.5f);
        virtual void draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes);
};
#endif // NANODET_H