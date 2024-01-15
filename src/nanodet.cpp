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
}

std::vector<BoxInfo> Nanodet::detect(cv::Mat &ocv_input)
{
    std::vector<BoxInfo> proposals;
    {
        ncnn::Mat input = ncnn::Mat::from_pixels_resize(ocv_input.data, ncnn::Mat::PIXEL_BGR, ocv_input.cols, ocv_input.rows, input_size[0], input_size[1]);
        
        // Preprocessing
        input.substract_mean_normalize(mean_vals, norm_vals);
        ncnn::Extractor ex = detector.create_extractor();
        ex.input("data", input);

        // Prediction
        // stride 8
        {
            ncnn::Mat cls_pred;
            ncnn::Mat dis_pred;
            ex.extract("cls8", cls_pred);
            ex.extract("dis8", dis_pred);

            std::vector<BoxInfo> obj8;
            generate_proposals(cls_pred, dis_pred, 8, input, 0.4, obj8);
            proposals.insert(proposals.end(), obj8.begin(), obj8.end());
        }

        // stride 16
        {
            ncnn::Mat cls_pred;
            ncnn::Mat dis_pred;
            ex.extract("cls16", cls_pred);
            ex.extract("dis16", dis_pred);

            std::vector<BoxInfo> obj16;
            generate_proposals(cls_pred, dis_pred, 16, input, 0.4, obj16);
            proposals.insert(proposals.end(), obj16.begin(), obj16.end());
        }

        // stride 32
        {
            ncnn::Mat cls_pred;
            ncnn::Mat dis_pred;
            ex.extract("cls32", cls_pred);
            ex.extract("dis32", dis_pred);

            std::vector<BoxInfo> obj32;
            generate_proposals(cls_pred, dis_pred, 32, input, 0.4, obj32);
            proposals.insert(proposals.end(), obj32.begin(), obj32.end());
        }

        // stride 64
        {
            ncnn::Mat cls_pred;
            ncnn::Mat dis_pred;
            ex.extract("cls64", cls_pred);
            ex.extract("dis64", dis_pred);

            std::vector<BoxInfo> obj64;
            generate_proposals(cls_pred, dis_pred, 64, input, 0.4, obj64);
            proposals.insert(proposals.end(), obj64.begin(), obj64.end());
        }
    }
    nms(proposals, 0.5);
    return proposals;
}

void Nanodet::draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes)
{
    float x_scale = (float) input.cols / input_size[0];
    float y_scale = (float) input.rows / input_size[1];
    draw_bboxes(input, boxxes, 0, x_scale, y_scale);
}