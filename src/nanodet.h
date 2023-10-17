#ifndef NANODET_H
#define NANODET_H

#include "net.h"
#include "simpleocv.h"
#include <vector>

class Nanodet
{
    ncnn::Net nanodet;
    // ncnn::Extractor extractor;
    // std::vector<char*>& input_names;
    // std::vector<char*>& output_names;

    public:
        // Nanodet(const char* param, const char* bin, const ncnn::Option &opt);
        virtual void create(const char* param, const char* bin, const ncnn::Option &opt);
        virtual void forward(ncnn::Mat &input);
        virtual void detect(const char* image);
        // virtual void detect();
        // virtual void nms();
};

#endif // NANODET_H