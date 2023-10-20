#include "nanodet.h"

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

static void generate_grid_center_priors(const int input_height, const int input_width, std::vector<int>& strides, std::vector<CenterPrior>& center_priors)
{
    for (int i = 0; i < (int)strides.size(); i++)
    {
        int stride = strides[i];
        int feat_w = ceil((float)input_width / stride);
        int feat_h = ceil((float)input_height / stride);
        for (int y = 0; y < feat_h; y++)
        {
            for (int x = 0; x < feat_w; x++)
            {
                CenterPrior ct;
                ct.x = x;
                ct.y = y;
                ct.stride = stride;
                center_priors.push_back(ct);
            }
        }
    }
}


int Nanodet::create(const char* param, const char* bin, const ncnn::Option &opt)
{
    int err_handler = 0;
    
    // Load setting
    nanodet.opt = opt;
    err_handler += nanodet.load_param(param);
    err_handler += nanodet.load_model(bin);

    // Initialize containers
    generate_grid_center_priors(this->input_size[0], this->input_size[1], this->strides, center_priors);

    if (err_handler < 0)
    {
        return -1;
    }
    return 0;
}

void Nanodet::inference_test(ncnn::Mat &input)
{
    input.substract_mean_normalize(mean_vals, norm_vals);

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

std::vector<BoxInfo> Nanodet::detect(ncnn::Mat &input)
{
    std::vector<std::vector<BoxInfo>> results;
    results.resize(num_class);

    // Preprocessing
    input.substract_mean_normalize(mean_vals, norm_vals);
    ncnn::Extractor ex = nanodet.create_extractor();
    ex.input("data", input);

    // Prediction
    ncnn::Mat out;
    ex.extract("output", out);
    decode(out, center_priors, 0.4, results);

    std::vector<BoxInfo> dets;
    for (int i = 0; i < (int)results.size(); i++)
    {
        this->nms(results[i], 0.5);

        for (auto box : results[i])
        {
            dets.push_back(box);
        }
    }
    return dets;
}

void Nanodet::decode(ncnn::Mat &output, std::vector<CenterPrior> &center_priors, float threshold, std::vector<std::vector<BoxInfo>> &results)
{
    int num_points = center_priors.size();
    float *score_ptr;

    for (int i = 0; i < num_points; i++)
    {
        int ctr_x = center_priors[i].x;
        int ctr_y = center_priors[i].y;
        int ctr_stride = center_priors[i].stride;

        // We have output in shape [1, 2212, 112]
        score_ptr = output.row(i); // point to the second dim
        float score = 0;
        int cur_label = 0;
        for (int label = 0; label < num_class; label++)
        {
            if (score_ptr[label] > score)
            {
                score = score_ptr[label];
                cur_label = label;
            }
        }

        if (score > threshold)
        {
            const float *bbox_pred = output.row(i) + this->num_class; // Point to the last 32 bytes of box predictions
            BoxInfo boxinfo = disPred2Bbox(bbox_pred, cur_label, score, ctr_x, ctr_y, ctr_stride);
            results[cur_label].push_back(boxinfo);
        }
    }
}

BoxInfo Nanodet::disPred2Bbox(const float *&dfl_det, int label, float score, int x, int y, int stride)
{
    float ct_x = x * stride;
    float ct_y = y * stride;

    std::vector<float> dis_pred;
    dis_pred.resize(4);
    for (int i = 0; i < 4; i++)
    {
        float dis = 0;
        float* dis_after_sm = new float[this->reg_max + 1];
        activation_function_softmax(dfl_det + i * (this->reg_max + 1), dis_after_sm, this->reg_max + 1);
        for (int j = 0; j < this->reg_max + 1; j++)
        {
            dis += j * dis_after_sm[j];
        }
        dis *= stride;
        //std::cout << "dis:" << dis << std::endl;
        dis_pred[i] = dis;
        delete[] dis_after_sm;
    }
    float xmin = (std::max)(ct_x - dis_pred[0], .0f);
    float ymin = (std::max)(ct_y - dis_pred[1], .0f);
    float xmax = (std::min)(ct_x + dis_pred[2], (float)this->input_size[0]);
    float ymax = (std::min)(ct_y + dis_pred[3], (float)this->input_size[1]);

    //std::cout << xmin << "," << ymin << "," << xmax << "," << xmax << "," << std::endl;
    return BoxInfo { xmin, ymin, xmax, ymax, score, label };
}

void Nanodet::nms(std::vector<BoxInfo>& input_boxes, float NMS_THRESH)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
            * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        for (int j = i + 1; j < int(input_boxes.size());) {
            float xx1 = (std::max)(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = (std::max)(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = (std::min)(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = (std::min)(input_boxes[i].y2, input_boxes[j].y2);
            float w = (std::max)(float(0), xx2 - xx1 + 1);
            float h = (std::max)(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH) {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else {
                j++;
            }
        }
    }
}