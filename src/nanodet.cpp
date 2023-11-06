#include "nanodet.h"

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

void Nanodet::load_param(const char* json_file)
{
    FILE* fp = fopen(json_file, "rb"); 
    char readBuffer[4000];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer)); 
    rapidjson::Document doc; 
    doc.ParseStream(is); 
    fclose(fp);

    input_size[0] = doc["config"]["input_shape"][0].GetInt();
    input_size[1] = doc["config"]["input_shape"][1].GetInt();

    for (auto &stride: doc["config"]["stride"].GetArray())
    {
        strides.push_back(stride.GetInt());
    }

    num_class = doc["config"]["num_classes"].GetInt();;
    reg_max = doc["config"]["reg_max"].GetInt();

    for (int i = 0; i < 3; i++)
    {
        mean_vals[i] = doc["config"]["mean_vals"][i].GetFloat();
        norm_vals[i] = doc["config"]["norm_vals"][i].GetFloat();
    }

    // NCNN opt
    ncnn::Option opt;
    opt.num_threads = doc["ncnn"]["num_threats"].GetInt();
    opt.use_winograd_convolution = doc["ncnn"]["winograd_convolution"].GetBool();
    opt.use_sgemm_convolution = doc["ncnn"]["sgemm_convolution"].GetBool();
    opt.use_int8_inference = doc["ncnn"]["int8_inference"].GetBool();

    create(doc["param"].GetString(), doc["bin"].GetString(), opt);
    generate_grid_center_priors(input_size[0], input_size[1], strides, center_priors);
}

std::vector<BoxInfo> Nanodet::detect(ncnn::Mat &input)
{
    std::vector<std::vector<BoxInfo>> results;
    results.resize(num_class);

    {
        // Preprocessing
        input.substract_mean_normalize(mean_vals, norm_vals);
        ncnn::Extractor ex = detector.create_extractor();

        ncnn::Mat resized(input_size[0], input_size[1], 3);
        bordered_resize(input, resized, input_size[0], 0);

        ex.input("data", resized);

        // Prediction
        ncnn::Mat out;
        ex.extract("output", out);
        decode(out, center_priors, 0.4, results);
    }

    std::vector<BoxInfo> dets;
    for (int i = 0; i < (int)results.size(); i++)
    {
        nms(results[i], 0.5);

        for (auto box : results[i])
        {
            dets.push_back(box);
        }
    }
    return dets;
}

void Nanodet::draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes)
{
    float scale = (float) input.cols / input_size[0];
    draw_bboxes(input, boxxes, 0, scale, scale);
}