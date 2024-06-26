#include <cmath>
#include "vision.h"
#include "fastestdet.h"

void FastestDet::load_param(const char *json_file)
{
    FILE                     *fp = fopen(json_file, "rb");
    char                      readBuffer[4000];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document       doc;
    doc.ParseStream(is);
    fclose(fp);

    input_size[0] = doc["config"]["input_shape"][0].GetInt();
    input_size[1] = doc["config"]["input_shape"][1].GetInt();

    num_class = doc["config"]["num_classes"].GetInt();
    ;

    for (int i = 0; i < 3; i++) {
        mean_vals[i] = doc["config"]["mean_vals"][i].GetFloat();
        norm_vals[i] = doc["config"]["norm_vals"][i].GetFloat();
    }

    blob_pool_allocator.clear();
    workspace_pool_allocator.clear();

    // NCNN opt
    ncnn::Option opt;
    opt.num_threads              = doc["ncnn"]["num_threats"].GetInt();
    opt.use_winograd_convolution = doc["ncnn"]["winograd_convolution"].GetBool();
    opt.use_sgemm_convolution    = doc["ncnn"]["sgemm_convolution"].GetBool();
    opt.use_int8_inference       = doc["ncnn"]["int8_inference"].GetBool();
    opt.blob_allocator           = &blob_pool_allocator;
    opt.workspace_allocator      = &workspace_pool_allocator;

    create(doc["param"].GetString(), doc["bin"].GetString(), opt);
}

std::vector<BoxInfo> FastestDet::detect(cv::Mat &ocv_input)
{
    ncnn::Mat out;
    {
        ncnn::Mat input = ncnn::Mat::from_pixels_resize(ocv_input.data, ncnn::Mat::PIXEL_BGR, ocv_input.cols,
                                                        ocv_input.rows, input_size[0], input_size[1]);

        input.substract_mean_normalize(mean_vals, norm_vals);
        ncnn::Extractor ex = detector.create_extractor();

        ex.input("data", input);
        ex.extract("output", out);
    }

    int                  c_step = out.cstep;
    float                obj_score;
    std::vector<BoxInfo> results;

    int count = 0;

    for (int h = 0; h < out.h; h++) {
        float *ptr = out.row(h);
        for (int w = 0; w < out.w; w++) {
            float obj_score     = ptr[0];
            float max_cls_score = 0.0;
            int   max_cls_idx   = -1;

            for (int c = 0; c < num_class; c++) {
                float cls_score = ptr[(c + 5) * c_step];
                if (cls_score > max_cls_score) {
                    max_cls_score = cls_score;
                    max_cls_idx   = c;
                }
            }

            if (pow(max_cls_score, 0.4) * pow(obj_score, 0.6) > 0.65) {
                float x_offset   = fast_tanh(ptr[c_step]);
                float y_offset   = fast_tanh(ptr[c_step * 2]);
                float box_width  = fast_sigmoid(ptr[c_step * 3]);
                float box_height = fast_sigmoid(ptr[c_step * 4]);
                float x_center   = (w + x_offset) / out.w;
                float y_center   = (h + y_offset) / out.h;

                BoxInfo info;
                info.x1    = (x_center - 0.5 * box_width) * ocv_input.cols;
                info.y1    = (y_center - 0.5 * box_height) * ocv_input.rows;
                info.x2    = (x_center + 0.5 * box_width) * ocv_input.cols;
                info.y2    = (y_center + 0.5 * box_height) * ocv_input.rows;
                info.label = max_cls_idx;
                info.score = obj_score;

                results.push_back(info);
            }
            ptr++;
        }
    }
    // Debug
    nms(results, 0.65);
    return results;
}

void FastestDet::draw_boxxes(cv::Mat &input, std::vector<BoxInfo> &boxxes) { draw_bboxes(input, boxxes, 0, 1, 1); }