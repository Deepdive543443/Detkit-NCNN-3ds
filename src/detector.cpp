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

inline float fast_sigmoid(float x)
{
    return 1.0f / (1.0f + fast_exp(-x));
}

inline float fast_tanh(float x)
{
    return 2.f / (1.f + fast_exp(-2 * x)) - 1.f;
}

int activation_function_softmax(const float *src, float *dst, int length)
{
    const float alpha = *std::max_element(src, src + length);
    float denominator = 0.f;

    for (int i = 0; i < length; ++i)
    {
        dst[i] = fast_exp(src[i] - alpha);
        denominator += dst[i];
    }

    for (int i = 0; i < length; ++i)
    {
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

void Detector::inference_test()
{
    ncnn::Mat input(input_size[0], input_size[1], 3);
    // input.substract_mean_normalize(mean_vals, norm_vals);

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

void Detector::nms(std::vector<BoxInfo>& input_boxes, float NMS_THRESH)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
            * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        for (int j = i + 1; j < int(input_boxes.size());)
        {
            float xx1 = (std::max)(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = (std::max)(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = (std::min)(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = (std::min)(input_boxes[i].y2, input_boxes[j].y2);
            float w = (std::max)(float(0), xx2 - xx1 + 1);
            float h = (std::max)(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH)
            {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else
            {
                j++;
            }
        }
    }
}