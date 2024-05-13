#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#if defined(MASSIVE_VIN_SCANNER_IOS)
#include "ncnn/ncnn/layer.h"
#include "ncnn/ncnn/net.h"
#else
#include "layer.h"
#include "net.h"
#include <stdlib.h>
#endif

#if defined(USE_NCNN_SIMPLEOCV)
#if defined(MASSIVE_VIN_SCANNER_IOS)
#include "ncnn/ncnn/simpleocv.h"
#else
#include "simpleocv.h"
#include "benchmark.h"
#include "cpu.h"
#include "datareader.h"
#include "net.h"
#include "gpu.h"
#endif
#else
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#endif



#include <float.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <cerrno>

#include <cmath> // Include for std::round

double roundToThreeDecimalPoints(double number) {
    return std::round(number * 1000.0) / 1000.0;
}

#include <cstdio> // Include for 'fprintf' and 'stderr'
using namespace cv;

#include <unordered_map>
#include <unordered_set>

namespace {


static const int num_class = 34;

static const char* class_names[] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", 
    "A", "B", "C", "D", "E", "F", "G", "H", "J", "K", 
    "L", "M", "N", "P", "R", "S", "T", "U", "V", "W", 
    "X", "Y", "Z", "vin"
    };

    

 static const unsigned char colors[81][3] = {
            {56,  0,   255},
            {226, 255, 0},
            {0,   94,  255},
            {0,   37,  255},
            {0,   255, 94},
            {255, 226, 0},
            {0,   18,  255},
            {255, 151, 0},
            {170, 0,   255},
            {0,   255, 56},
            {255, 0,   75},
            {0,   75,  255},
            {0,   255, 169},
            {255, 0,   207},
            {75,  255, 0},
            {207, 0,   255},
            {37,  0,   255},
            {0,   207, 255},
            {94,  0,   255},
            {0,   255, 113},
            {255, 18,  0},
            {255, 0,   56},
            {18,  0,   255},
            {0,   255, 226},
            {170, 255, 0},
            {255, 0,   245},
            {151, 255, 0},
            {132, 255, 0},
            {75,  0,   255},
            {151, 0,   255},
            {0,   151, 255},
            {132, 0,   255},
            {0,   255, 245},
            {255, 132, 0},
            {226, 0,   255},
            {255, 37,  0},
            {207, 255, 0},
            {0,   255, 207},
            {94,  255, 0},
            {0,   226, 255},
            {56,  255, 0},
            {255, 94,  0},
            {255, 113, 0},
            {0,   132, 255},
            {255, 0,   132},
            {255, 170, 0},
            {255, 0,   188},
            {113, 255, 0},
            {245, 0,   255},
            {113, 0,   255},
            {255, 188, 0},
            {0,   113, 255},
            {255, 0,   0},
            {0,   56,  255},
            {255, 0,   113},
            {0,   255, 188},
            {255, 0,   94},
            {255, 0,   18},
            {18,  255, 0},
            {0,   255, 132},
            {0,   188, 255},
            {0,   245, 255},
            {0,   169, 255},
            {37,  255, 0},
            {255, 0,   151},
            {188, 0,   255},
            {0,   255, 37},
            {0,   255, 0},
            {255, 0,   170},
            {255, 0,   37},
            {255, 75,  0},
            {0,   0,   255},
            {255, 207, 0},
            {255, 0,   226},
            {255, 245, 0},
            {188, 255, 0},
            {0,   255, 18},
            {0,   255, 75},
            {0,   255, 151},
            {255, 56,  0},
            {245, 255, 0}
    };

}

ncnn::Net yolov8;


void initYolo8(char *modelPath, char *paramPath)
{

    yolov8.load_param(paramPath);
    yolov8.load_model(modelPath);
    
}

void disposeYolo8()
{
    yolov8.clear();
}
static void slice(const ncnn::Mat& in, ncnn::Mat& out, int start, int end, int axis)
{
    ncnn::Option opt;
    opt.num_threads = 4;
    opt.use_fp16_storage = false;
    opt.use_packing_layout = false;

    ncnn::Layer* op = ncnn::create_layer("Crop");

    ncnn::ParamDict pd;

    ncnn::Mat axes = ncnn::Mat(1);
    axes.fill(axis);
    ncnn::Mat ends = ncnn::Mat(1);
    ends.fill(end);
    ncnn::Mat starts = ncnn::Mat(1);
    starts.fill(start);
    pd.set(9, starts);
    pd.set(10, ends);
    pd.set(11, axes);

    op->load_param(pd);

    op->create_pipeline(opt);

    op->forward(in, out, opt);

    op->destroy_pipeline(opt);

    delete op;
}


static void interp(const ncnn::Mat& in, const float& scale, const int& out_w, const int& out_h, ncnn::Mat& out)
{
    ncnn::Option opt;
    opt.num_threads = 4;
    opt.use_fp16_storage = false;
    opt.use_packing_layout = false;

    ncnn::Layer* op = ncnn::create_layer("Interp");

   
    ncnn::ParamDict pd;
    pd.set(0, 2);
    pd.set(1, scale);
    pd.set(2, scale);
    pd.set(3, out_h);
    pd.set(4, out_w);

    op->load_param(pd);

    op->create_pipeline(opt);


    op->forward(in, out, opt);

    op->destroy_pipeline(opt);

    delete op;
}
static void reshape(const ncnn::Mat& in, ncnn::Mat& out, int c, int h, int w, int d)
{
    ncnn::Option opt;
    opt.num_threads = 4;
    opt.use_fp16_storage = false;
    opt.use_packing_layout = false;

    ncnn::Layer* op = ncnn::create_layer("Reshape");

 
    ncnn::ParamDict pd;

    pd.set(0, w);
    pd.set(1, h);
    if (d > 0)
        pd.set(11, d);
    pd.set(2, c);
    op->load_param(pd);

    op->create_pipeline(opt);
    op->forward(in, out, opt);

    op->destroy_pipeline(opt);

    delete op;
}
static void sigmoid(ncnn::Mat& bottom)
{
    ncnn::Option opt;
    opt.num_threads = 4;
    opt.use_fp16_storage = false;
    opt.use_packing_layout = false;

    ncnn::Layer* op = ncnn::create_layer("Sigmoid");

    op->create_pipeline(opt);


    op->forward_inplace(bottom, opt);
    op->destroy_pipeline(opt);

    delete op;
}
static void matmul(const std::vector<ncnn::Mat>& bottom_blobs, ncnn::Mat& top_blob)
{
    ncnn::Option opt;
    opt.num_threads = 2;
    opt.use_fp16_storage = false;
    opt.use_packing_layout = false;

    ncnn::Layer* op = ncnn::create_layer("MatMul");


    ncnn::ParamDict pd;
    pd.set(0, 0);

    op->load_param(pd);

    op->create_pipeline(opt);
    std::vector<ncnn::Mat> top_blobs(1);
    op->forward(bottom_blobs, top_blobs, opt);
    top_blob = top_blobs[0];

    op->destroy_pipeline(opt);

    delete op;
}

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
    cv::Mat mask;
    std::vector<float> mask_feat;
};
struct GridAndStride
{
    int grid0;
    int grid1;
    int stride;
};
static inline float intersection_area(const Object& a, const Object& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
        #pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects)
{
    if (faceobjects.empty())
        return;

    qsort_descent_inplace(faceobjects, 0, faceobjects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold)
{
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = faceobjects[i].rect.width * faceobjects[i].rect.height;
    }

    for (int i = 0; i < n; i++)
    {
        const Object& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const Object& b = faceobjects[picked[j]];

          
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}
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
static void generate_proposals(std::vector<GridAndStride> grid_strides, const ncnn::Mat& pred, float prob_threshold, std::vector<Object>& objects)
{
    const int num_points = grid_strides.size();
  
    const int num_class = 34;
    const int reg_max_1 = 16;

    for (int i = 0; i < num_points; i++)
    {
            const float* scores = pred.row(i) + 4 * reg_max_1;

      
            int label = -1;
            float score = -FLT_MAX;
            for (int k = 0; k < num_class; k++)
            {
                float confidence = scores[k];
                if (confidence > score)
                {
                    label = k;
                    score = confidence;
                }
            }
            float box_prob = sigmoid(score);
            if (box_prob >= prob_threshold)
            {
                ncnn::Mat bbox_pred(reg_max_1, 4, (void*)pred.row(i));
                {
                    ncnn::Layer* softmax = ncnn::create_layer("Softmax");

                    ncnn::ParamDict pd;
                    pd.set(0, 1);
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

                    pred_ltrb[k] = dis * grid_strides[i].stride;
                }

                float pb_cx = (grid_strides[i].grid0 + 0.5f) * grid_strides[i].stride;
                float pb_cy = (grid_strides[i].grid1 + 0.5f) * grid_strides[i].stride;

                float x0 = pb_cx - pred_ltrb[0];
                float y0 = pb_cy - pred_ltrb[1];
                float x1 = pb_cx + pred_ltrb[2];
                float y1 = pb_cy + pred_ltrb[3];

                Object obj;
                obj.rect.x = x0;
                obj.rect.y = y0;
                obj.rect.width = x1 - x0;
                obj.rect.height = y1 - y0;
                obj.label = label;
                obj.prob = box_prob;
                obj.mask_feat.resize(32);
                std::copy(pred.row(i) + 64 + num_class, pred.row(i) + 64 + num_class + 32, obj.mask_feat.begin());
                objects.push_back(obj);
            }
    }
}
static void generate_grids_and_stride(const int target_w, const int target_h, std::vector<int>& strides, std::vector<GridAndStride>& grid_strides)
{
    for (int i = 0; i < (int)strides.size(); i++)
    {
        int stride = strides[i];
        int num_grid_w = target_w / stride;
        int num_grid_h = target_h / stride;
        for (int g1 = 0; g1 < num_grid_h; g1++)
        {
            for (int g0 = 0; g0 < num_grid_w; g0++)
            {
                GridAndStride gs;
                gs.grid0 = g0;
                gs.grid1 = g1;
                gs.stride = stride;
                grid_strides.push_back(gs);
            }
        }
    }
}
static void decode_mask(const ncnn::Mat& mask_feat, const int& img_w, const int& img_h,
    const ncnn::Mat& mask_proto, const ncnn::Mat& in_pad, const int& wpad, const int& hpad,
    ncnn::Mat& mask_pred_result)
{
    ncnn::Mat masks;
    matmul(std::vector<ncnn::Mat>{mask_feat, mask_proto}, masks);
    sigmoid(masks);
    reshape(masks, masks, masks.h, in_pad.h / 4, in_pad.w / 4, 0);
    slice(masks, mask_pred_result, (wpad / 2) / 4, (in_pad.w - wpad / 2) / 4, 2);
    slice(mask_pred_result, mask_pred_result, (hpad / 2) / 4, (in_pad.h - hpad / 2) / 4, 1);
    interp(mask_pred_result, 4.0, img_w, img_h, mask_pred_result);

}




int detect_yolov8(cv::Mat& bgr, std::vector<Object>& objects)
{
   

    std::vector<cv::Rect> bounding_boxes;




    int width = bgr.cols;
    int height = bgr.rows;

    const int target_size = 640;
    const float prob_threshold = 0.4f;   //  0.4f
    const float nms_threshold = 0.5f;      //0.5f


    

    int w = width;
    int h = height;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)target_size / w;
        w = target_size;
        h = h * scale;
    }
    else
    {
        scale = (float)target_size / h;
        h = target_size;
        w = w * scale;
    }

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR2RGB, width, height, w, h);


    int wpad = (w + 31) / 32 * 32 - w;
    int hpad = (h + 31) / 32 * 32 - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT, 0.f);

    const float norm_vals[3] = { 1 / 255.f, 1 / 255.f, 1 / 255.f };
    in_pad.substract_mean_normalize(0, norm_vals);


    ncnn::Extractor ex = yolov8.create_extractor();
    ex.input("images", in_pad);

    ncnn::Mat out;
    ex.extract("output0", out);

  
    std::vector<int> strides = { 8, 16, 32 };
    std::vector<GridAndStride> grid_strides;
    generate_grids_and_stride(in_pad.w, in_pad.h, strides, grid_strides);

    std::vector<Object> proposals;
    std::vector<Object> objects8;
    generate_proposals(grid_strides, out, prob_threshold, objects8);
    
    proposals.insert(proposals.end(), objects8.begin(), objects8.end());


    qsort_descent_inplace(proposals);


    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked, nms_threshold);

    int count = picked.size();


    objects.resize(count);
    for (int i = 0; i < count; i++)
    {
        objects[i] = proposals[picked[i]];


        float x0 = (objects[i].rect.x - (wpad / 2)) / scale;
        float y0 = (objects[i].rect.y - (hpad / 2)) / scale;
        float x1 = (objects[i].rect.x + objects[i].rect.width - (wpad / 2)) / scale;
        float y1 = (objects[i].rect.y + objects[i].rect.height - (hpad / 2)) / scale;

       
        x0 = std::max(std::min(x0, (float)(width - 1)), 0.f);
        y0 = std::max(std::min(y0, (float)(height - 1)), 0.f);
        x1 = std::max(std::min(x1, (float)(width - 1)), 0.f);
        y1 = std::max(std::min(y1, (float)(height - 1)), 0.f);

        objects[i].rect.x = x0;
        objects[i].rect.y = y0;
        objects[i].rect.width = x1 - x0;
        objects[i].rect.height = y1 - y0;

       

       cv::Rect bbox(x0, y0, x1 - x0, y1 - y0);
        bounding_boxes.push_back(bbox);
    }

    // Convert the bounding boxes to a vector of uchar
    std::vector<uchar> retv;

    for (const cv::Rect& bbox : bounding_boxes)
    {
        const uchar* data = reinterpret_cast<const uchar*>(&bbox);

        for (size_t i = 0; i < sizeof(cv::Rect); i++)
        {
            retv.push_back(data[i]);
        }
    }

  
    return 0;
}

static void draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects)
{

        char text[256];
      

    static const char* class_names[] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", 
    "A", "B", "C", "D", "E", "F", "G", "H", "J", "K", 
    "L", "M", "N", "P", "R", "S", "T", "U", "V", "W", 
    "X", "Y", "Z", "vin"
};

    static const unsigned char colors[81][3] = {
            {243, 255, 7},
            {226, 255, 0},
            {0,   94,  255},
            {0,   37,  255},
            {0,   255, 94},
            {243, 255, 7},
            {0,   18,  255},
            {255, 151, 0},
            {170, 0,   255},
            {0,   255, 56},
            {255, 0,   75},
            {0,   75,  255},
            {0,   255, 169},
            {243, 255, 7},
            {75,  255, 0},
            {207, 0,   255},
            {37,  0,   255},
            {0,   207, 255},
            {94,  0,   255},
            {0,   255, 113},
            {255, 18,  0},
            {255, 0,   56},
            {18,  0,   255},
            {0,   255, 226},
            {170, 255, 0},
            {255, 0,   245},
            {151, 255, 0},
            {132, 255, 0},
            {75,  0,   255},
            {151, 0,   255},
            {0,   151, 255},
            {132, 0,   255},
            {0,   255, 245},
            {255, 132, 0},
            {226, 0,   255},
            {255, 37,  0},
            {207, 255, 0},
            {0,   255, 207},
            {94,  255, 0},
            {0,   226, 255},
            {56,  255, 0},
            {255, 94,  0},
            {255, 113, 0},
            {0,   132, 255},
            {255, 0,   132},
            {255, 170, 0},
            {255, 0,   188},
            {113, 255, 0},
            {245, 0,   255},
            {113, 0,   255},
            {255, 188, 0},
            {0,   113, 255},
            {255, 0,   0},
            {0,   56,  255},
            {255, 0,   113},
            {0,   255, 188},
            {255, 0,   94},
            {255, 0,   18},
            {18,  255, 0},
            {0,   255, 132},
            {0,   188, 255},
            {0,   245, 255},
            {0,   169, 255},
            {37,  255, 0},
            {255, 0,   151},
            {188, 0,   255},
            {0,   255, 37},
            {0,   255, 0},
            {255, 0,   170},
            {255, 0,   37},
            {255, 75,  0},
            {0,   0,   255},
            {255, 207, 0},
            {255, 0,   226},
            {255, 245, 0},
            {188, 255, 0},
            {0,   255, 18},
            {0,   255, 75},
            {0,   255, 151},
            {255, 56,  0},
            {245, 255, 0}
    };
    cv::Mat image = bgr;
    int color_index = 0;
    
    for (size_t i = 0; i < objects.size(); i++)
    {
        const Object& obj = objects[i];
        const unsigned char* color = colors[color_index % 80];
        color_index++;

        cv::Scalar cc(color[0], color[1], color[2]);

        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
                obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);
        
        sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > image.cols)
            x = image.cols - label_size.width;

        cv::rectangle(image, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                      cv::Scalar(255, 255, 255), -1);

        cv::putText(image, text, cv::Point(x, y + label_size.height),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
                   
    }


    
}

std::vector<int32_t> getCombinedObjectResults(const std::vector<Object>& objects, int img_width, int img_height)
{
    std::vector<int> combinedResults;

    for (size_t i = 0; i < objects.size(); ++i)
    {
        const auto& obj = objects[i];

        if (i != 0)
        {
            combinedResults.push_back(static_cast<int>(-1));
        }

  
        int cx = obj.rect.x;
        int cy = obj.rect.y;
        int bw = obj.rect.width;
        int bh = obj.rect.height;
        double prob = obj.prob;
        double prob_threshold=roundToThreeDecimalPoints(prob) ;
        int int_prob = int(1000*prob_threshold) ;

    
        combinedResults.push_back(static_cast<int>(obj.label));
        combinedResults.push_back(cx);
        combinedResults.push_back(cy);
        combinedResults.push_back(bw);
        combinedResults.push_back(bh);
        combinedResults.push_back(int_prob);


      
        if (i != objects.size() - 1)
        {
            combinedResults.push_back(static_cast<int>(-1));
        }
    }

    return combinedResults;
}





template<typename T>
T clamp(T val, T minVal, T maxVal) {
    return (val < minVal) ? minVal : (val > maxVal) ? maxVal : val;
}
// Convert YUV420 to RGB image
void convertYUV420ToImage(int width, int height, const std::vector<uint8_t>& bytes, int bytesPerRow, cv::Mat& rgbaImage) {
    rgbaImage.create(height, width, CV_8UC4); // Create an image with 4 channels
    int uvRowStride = bytesPerRow / 2;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int yIndex = y * bytesPerRow + x;
            int uvIndex = (y / 2) * uvRowStride + (x / 2);
            uint8_t Y = bytes[yIndex];
            uint8_t U = bytes[uvIndex];
            uint8_t V = bytes[uvIndex + 1];

            int R = clamp(int(Y + (V * 1436 / 1024 - 179)), 0, 255);
            int G = clamp(int(Y - (U * 46549 / 131072 + 44 - V * 93604 / 131072 + 91)), 0, 255);
            int B = clamp(int(Y + (U * 1814 / 1024 - 227)), 0, 255);
            int A = 255;  // Alpha channel set to full opacity

            rgbaImage.at<cv::Vec4b>(y, x) = cv::Vec4b(B, G, R, A);
        }
    }
}
 extern "C" __attribute__((visibility("default"))) __attribute__((used)) void image_ffi_yuv24(unsigned char* buf, int size,int width,int height, int* segBoundary, int* segBoundarySize) {
    int bytesPerRow = width; // Assuming the row stride equals the width
    cv::Mat rgbImage;
    std::vector<uint8_t> bytes(buf, buf + size); // Assuming size is correctly computed for the YUV data
    convertYUV420ToImage(width, height, bytes, bytesPerRow, rgbImage);
    std::vector<Object> objects;
    detect_yolov8(rgbImage, objects);
    std::vector<int32_t> results = getCombinedObjectResults(objects, rgbImage.cols, rgbImage.rows);
    size_t resultsSize = results.size()*sizeof(int32_t);
    if (resultsSize > size) {
        return;
    }
    memcpy(segBoundary, results.data(), resultsSize);
    *segBoundarySize = results.size() ;
}




void image_ffi(unsigned char* buf,  int size , int* segBoundary , int* segBoundarySize ) { 
    std::vector<uchar> v(buf, buf + size);
    cv::Mat receivedImage = cv::imdecode(cv::Mat(v), cv::IMREAD_COLOR);

    // Check if the image is in portrait mode and rotate it to landscape
    if (receivedImage.rows < receivedImage.cols) {
        cv::rotate(receivedImage, receivedImage, cv::ROTATE_90_CLOCKWISE);
    }
     
    std::vector<Object> objects;
    std::vector<uchar> retv;
    detect_yolov8(receivedImage, objects);
  
    std::vector<int32_t> results = getCombinedObjectResults(objects, receivedImage.cols, receivedImage.rows);

    size_t resultsSize = results.size()*sizeof(int32_t);

    

    if (resultsSize > size) {
        return;
    }

    memcpy(segBoundary, results.data(), resultsSize);

    

    *segBoundarySize = results.size() ;




}
void image_ffi_path(char *path, int* objectCnt) { 
    cv::Mat recievedImage = cv::imread(path);
    Mat temp;
    std::vector<Object> objects;
    std::vector <uchar> retv;
    detect_yolov8(recievedImage, objects);
    *objectCnt =  objects.size();
    draw_objects(recievedImage, objects);
    std::vector <uchar> retv1;
    imwrite(path, recievedImage);
   
}
// Function to convert BGRA8888 to BGR using OpenCV
void ConvertBGRA8888toBGR(const cv::Mat& bgraImage, cv::Mat& bgrImage) {
    if (bgraImage.type() != CV_8UC4) {
        std::cerr << "Invalid input image format: Expected CV_8UC4 (BGRA8888)" << std::endl;
        return;
    }
    cv::cvtColor(bgraImage, bgrImage, cv::COLOR_BGRA2BGR);
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void image_ffi_bgra8888(unsigned char* buf, int size, int width, int height, int* segBoundary, int* segBoundarySize) {
    // Create an OpenCV mat that references the BGRA8888 data
    cv::Mat bgraImage(height, width, CV_8UC4, buf);
    cv::Mat bgrImage;

    // Convert from BGRA8888 to BGR
    ConvertBGRA8888toBGR(bgraImage, bgrImage);

    // Process the BGR image using your object detection function
    std::vector<Object> objects;
    detect_yolov8(bgrImage, objects);

    // Get the combined results from the detected objects
    std::vector<int32_t> results = getCombinedObjectResults(objects, bgrImage.cols, bgrImage.rows);
    size_t resultsSize = results.size() * sizeof(int32_t);

    // Ensure the output buffer is large enough
    if (resultsSize > static_cast<size_t>(size)) {
        *segBoundarySize = 0; // Not enough space to store results
        return;
    }

    // Copy the results to the provided buffer
    memcpy(segBoundary, results.data(), resultsSize);
    *segBoundarySize = results.size(); // Update the number of elements in the output array
}




