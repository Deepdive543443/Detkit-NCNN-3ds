#include "detector.h"

inline float fast_exp(float x)
{
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
    return v.f;
}

inline float sigmoid(float x)
{
    return 1.0f / (1.0f + fast_exp(-x));
}

int activation_function_softmax(const float *src, float *dst, int length)
{
    const float alpha = *std::max_element(src, src + length);
    float denominator = 0.f;

    for (int i = 0; i < length; ++i) {
        dst[i] = fast_exp(src[i] - alpha);
        denominator += dst[i];
    }

    for (int i = 0; i < length; ++i) {
        dst[i] /= denominator;
    }
    return 0;
}


int Detector::create(const char* param, const char* bin, const ncnn::Option &opt)
{
    int err_handler = 0;
    
    // Load setting
    detector.opt = opt;
    err_handler += detector.load_param(param);
    err_handler += detector.load_model(bin);

    // Initialize containers
    if (err_handler < 0)
    {
        return -1;
    }
    return 0;
}

void Detector::inference_test(ncnn::Mat &input)
{
    input.substract_mean_normalize(mean_vals, norm_vals);

    const std::vector<const char*>& input_names = detector.input_names();
    const std::vector<const char*>& output_names = detector.output_names();
    ncnn::Extractor extractor = detector.create_extractor();

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