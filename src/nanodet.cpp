#include "nanodet.h"

int Nanodet::create(const char* param, const char* bin, const ncnn::Option &opt)
{
    int err_handler = 0;
    
    // Load setting
    nanodet.opt = opt;
    err_handler += nanodet.load_param(param);
    err_handler += nanodet.load_model(bin);

    if (err_handler < 0)
    {
        return -1;
    }
    return 0;
}

void Nanodet::forward_test(ncnn::Mat &input)
{
    const std::vector<const char*>& input_names = nanodet.input_names();
    const std::vector<const char*>& output_names = nanodet.output_names();
    ncnn::Extractor extractor = nanodet.create_extractor();

    printf("=======================\n");
    for (auto name : input_names)
    {
        printf("in_name: %s\n", name);
        extractor.input(name, input);
    }
    
    
    for (auto name : output_names)
    {
        ncnn::Mat out;
        extractor.extract(name, out);
        printf("out_name: %s\nShape: C: %d H: %d W: %d\n\n", name, out.c, out.h, out.w);
    }
    printf("=======================\n");
}

void Nanodet::detect(const char* image)
{
    printf("feee");
}

