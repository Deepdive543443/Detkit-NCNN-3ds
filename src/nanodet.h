#ifndef NANODET_H
#define NANODET_H

#include "detector.h"
#include "net.h"
#include "vision.h"

class Nanodet : public Detector {
   public:
    virtual void                 load_param(const char *json_file);
    virtual std::vector<BoxInfo> detect(cv::Mat &ocv_input);
    virtual void                 draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes);
};
#endif  // NANODET_H