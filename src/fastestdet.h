#ifndef FASTEST_DET_H
#define FASTEST_DET_H

#include "detector.h"
#include "net.h"
#include "vision.h"
#include <cmath>
#include "vision.h"

class FastestDet : public Detector
{
    public:
        virtual void load_param(const char* json_file);
        virtual std::vector<BoxInfo> detect(ncnn::Mat &input);
        virtual void draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes);
};
#endif // FASTEST_DET_H