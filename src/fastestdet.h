#ifndef FASTEST_DET_H
#define FASTEST_DET_H

#include "detector.h"
#include "net.h"
#include "vision.h"

class FastestDet : public Detector {
   public:
    virtual void                 load_param(const char *json_file);
    virtual std::vector<BoxInfo> detect(cv::Mat &ocv_input);
    virtual void                 draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes);

    int num_class;
};
#endif  // FASTEST_DET_H