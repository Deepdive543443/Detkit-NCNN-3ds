#include "nanodet.h"

static void generate_proposals(ncnn::Mat& cls_pred, ncnn::Mat& dis_pred, int stride, const ncnn::Mat& in_pad, float prob_threshold, std::vector<BoxInfo>& objects)
{
    const int num_grid_x = cls_pred.w;
    const int num_grid_y = cls_pred.h;
    const int num_class = cls_pred.c;
    const int cstep_cls = cls_pred.cstep;

    const int reg_max_1 = dis_pred.w / 4;
    const int hstep_dis = dis_pred.cstep;

    for (int i = 0; i < num_grid_y; i++)
    {
        for (int j = 0; j < num_grid_x; j++)
        {
            float *score_ptr = cls_pred.row(i) + j;
            float max_score = -FLT_MAX;
            int max_label = -1;

            for (int cls = 0; cls < num_class; cls++)
            {
                if (score_ptr[cls * cstep_cls] > max_score)
                {
                    max_score = score_ptr[cls * cstep_cls];
                    max_label = cls;
                }
            }

            if (max_score >= prob_threshold)
            {
                ncnn::Mat bbox_pred(reg_max_1, 4, (void*) (dis_pred.row(j) + i * hstep_dis));
                {
                    ncnn::Layer* softmax = ncnn::create_layer("Softmax");

                    ncnn::ParamDict pd;
                    pd.set(0, 1); // axis
                    pd.set(1, 1);
                    softmax->load_param(pd);

                    ncnn::Option opt;
                    opt.num_threads = 1;
                    opt.use_packing_layout = false;

                    softmax->create_pipeline(opt);
                    softmax->forward_inplace(bbox_pred, opt);
                    softmax->destroy_pipeline(opt);

                    delete softmax;
                }

                float pred_ltrb[4];
                for (int k = 0; k < 4; k++)
                {
                    float dis = 0.f;
                    const float* dis_after_sm = bbox_pred.row(k);
                    for (int l = 0; l < reg_max_1; l++)
                    {
                        dis += l * dis_after_sm[l];
                    }
                    pred_ltrb[k] = dis * stride;
                }

                float x_center = j * stride;
                float y_center = i * stride;

                // Object obj;
                // obj.rect.x = x_center - pred_ltrb[0];
                // obj.rect.y = y_center - pred_ltrb[1];
                // obj.rect.width =  pred_ltrb[2] + pred_ltrb[0];
                // obj.rect.height = pred_ltrb[3] + pred_ltrb[1];
                // obj.label = max_label;
                // obj.prob = max_score;
                BoxInfo obj;
                obj.x1 = x_center - pred_ltrb[0];
                obj.y1 = y_center - pred_ltrb[1];
                obj.x2 = x_center + pred_ltrb[2];
                obj.y2 = y_center + pred_ltrb[3];
                obj.score = max_score;
                obj.label = max_label;
                objects.push_back(obj);
            }
        }
    }
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

std::vector<BoxInfo> Nanodet::detect(cv::Mat &ocv_input, float prob_threshold, float nms_threshold)
{
    // std::vector<std::vector<BoxInfo>> results;
    // results.resize(num_class);
    std::vector<BoxInfo> proposals;
    {
        ncnn::Mat input = ncnn::Mat::from_pixels_resize(ocv_input.data, ncnn::Mat::PIXEL_RGB2BGR, ocv_input.cols, ocv_input.rows, 320, 192);
        
        // Preprocessing
        input.substract_mean_normalize(mean_vals, norm_vals);
        ncnn::Extractor ex = detector.create_extractor();
        ex.input("data", input);

        // Prediction
        // std::vector<BoxInfo> proposals;
        // stride 8
        {
            ncnn::Mat cls_pred;
            ncnn::Mat dis_pred;
            ex.extract("cls8", cls_pred);
            ex.extract("dis8", dis_pred);

            std::vector<BoxInfo> obj8;
            generate_proposals(cls_pred, dis_pred, 8, input, prob_threshold, obj8);
            proposals.insert(proposals.end(), obj8.begin(), obj8.end());
        }

        // stride 16
        {
            ncnn::Mat cls_pred;
            ncnn::Mat dis_pred;
            ex.extract("cls16", cls_pred);
            ex.extract("dis16", dis_pred);

            std::vector<BoxInfo> obj16;
            generate_proposals(cls_pred, dis_pred, 16, input, prob_threshold, obj16);
            proposals.insert(proposals.end(), obj16.begin(), obj16.end());
        }

        // stride 32
        {
            ncnn::Mat cls_pred;
            ncnn::Mat dis_pred;
            ex.extract("cls32", cls_pred);
            ex.extract("dis32", dis_pred);

            std::vector<BoxInfo> obj32;
            generate_proposals(cls_pred, dis_pred, 32, input, prob_threshold, obj32);
            proposals.insert(proposals.end(), obj32.begin(), obj32.end());
        }

        // stride 64
        {
            ncnn::Mat cls_pred;
            ncnn::Mat dis_pred;
            ex.extract("cls64", cls_pred);
            ex.extract("dis64", dis_pred);

            std::vector<BoxInfo> obj64;
            generate_proposals(cls_pred, dis_pred, 64, input, prob_threshold, obj64);
            proposals.insert(proposals.end(), obj64.begin(), obj64.end());
        }

    }
    nms(proposals, 0.5);
    // std::vector<BoxInfo> dets;
    // for (int i = 0; i < (int)results.size(); i++)
    // {
    //     nms(results[i], 0.5);

    //     for (auto box : results[i])
    //     {
    //         dets.push_back(box);
    //     }
    // }
    return proposals;
}

void Nanodet::draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes)
{
    float scale = (float) input.cols / input_size[0];
    draw_bboxes(input, boxxes, 0, scale, scale);
}