#include "nanodet.h"


// Nanodet::Nanodet(const char* param, const char* bin, const ncnn::Option &opt)
// {
//     // Load setting
//     nanodet.opt = opt;

//     // Load model and parameters
//     nanodet.load_param(param);
//     nanodet.load_model(bin);
// }

void Nanodet::create(const char* param, const char* bin, const ncnn::Option &opt)
{
    // Load setting
    nanodet.opt = opt;

    // Load model and parameters
    printf("Loaded %d", nanodet.load_param(param));
    printf("Loaded %d", nanodet.load_model(bin));
    
    // nanodet.load_param(param);
    // nanodet.load_model(bin);
}

void Nanodet::forward(ncnn::Mat &input)
{
    // for (auto name : input_names)
    // {
    //     printf("name: %s", name);
    // }
    const std::vector<const char*>& input_names = nanodet.input_names();
    const std::vector<const char*>& output_names = nanodet.output_names();
    ncnn::Extractor extractor = nanodet.create_extractor();;

    for (auto name : input_names)
    {
        printf("in_name: %s\n", name);
        extractor.input(name, input);
    }
    
    
    for (auto name : output_names)
    {
        ncnn::Mat out;
        extractor.extract(name, out);
        printf("out_name: %s Shape: C: %d H: %d W: %d\n", name, out.c, out.h, out.w);
    }
}

void Nanodet::detect(const char* image)
{
    printf("feee");
}

