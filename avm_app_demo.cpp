/***
 * function: 360 surrond view combine c++ demo
 * author: joker.mao
 * date: 2023/07/15
 * copyright: ADAS_EYES all right reserved
*/

#include "common.h"

//#define DEBUG
#define AWB_ENALE    1
#define LUM_BANLANCE 1

static inline uint8_t clip(float data, int max)
{
    if (data > max)
        return max;
    return (uint8_t)data;
}

static void merge_image(cv::Mat& src1, cv::Mat& src2, cv::Mat& w, cv::Mat& out)
{
    if (src1.size() != src2.size()) {
        return;
    }

    int p_index = 0;
    float* weights = (float *)(w.data);
    for (int h = 0; h < src1.rows; ++h) {
        uchar* p1 = src1.data + h * src1.step;
        uchar* p2 = src2.data + h * src2.step;
        uchar*  o = out.data + h * out.step;
        for (int w = 0; w < src1.cols; ++w) {
            o[0] = clip(p1[0] * weights[p_index] + p2[0] * (1 - weights[p_index]), 255);
            o[1] = clip(p1[1] * weights[p_index] + p2[1] * (1 - weights[p_index]), 255);
            o[2] = clip(p1[2] * weights[p_index] + p2[2] * (1 - weights[p_index]), 255);
            p1 += 3;
            p2 += 3;
            o  += 3;
            ++p_index;
        }
    }
}

int main(int argc, char** argv)
{
    std::cout  << argv[0] << " app start running..." << std::endl;
    cv::Mat car_img;
    cv::Mat origin_dir_img[4];
    cv::Mat undist_dir_img[4];
    cv::Mat merge_weights_img[4];
    cv::Mat out_put_img;
    float *w_ptr[4];

    //1.read image and read weights
    car_img = cv::imread("../../images/car.png");
    cv::resize(car_img, car_img, cv::Size(xr - xl, yb - yt));
    out_put_img = cv::Mat(cv::Size(total_w, total_h), CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat weights = cv::imread("../../yaml/weights.png", -1);

    if (weights.channels() != 4) {
        std::cerr << "imread weights failed " << weights.channels() << "\r\n";
        return -1;
    }

    for (int i = 0; i < 4; ++i) {
        merge_weights_img[i] = cv::Mat(weights.size(), CV_32FC1, cv::Scalar(0, 0, 0));
        w_ptr[i] = (float *)merge_weights_img[i].data;
    }
    int pixel_index = 0;

    for (int h = 0; h < weights.rows; ++h) {
        uchar* uc_pixel = weights.data + h * weights.step;
        for (int w = 0; w < weights.cols; ++w) {
            w_ptr[0][pixel_index] = uc_pixel[0] / 255.0f;
            w_ptr[1][pixel_index] = uc_pixel[1] / 255.0f;
            w_ptr[2][pixel_index] = uc_pixel[2] / 255.0f;
            w_ptr[3][pixel_index] = uc_pixel[3] / 255.0f;
            uc_pixel += 4;
            ++pixel_index;
        }
    }

#ifdef DEBUG
    for (int i = 0; i < 4; ++i) {
        //0 左下 1 右上 2 左上 3 左下
        display_mat(merge_weights_img[i], "w");
    }
#endif

    //2. undistort image
    for (int i = 0; i < 4; ++i) {
        camera_prms prms;
        prms.name = camera_names[i];
        origin_dir_img[i] = cv::imread("../../images/" + prms.name + ".png");
        cv::Mat& src = origin_dir_img[i];
       
        read_prms("../../yaml/" + prms.name + ".yaml", prms);
        undist_by_remap(src, src, prms);
        cv::warpPerspective(src, src, prms.project_matrix, project_shapes[prms.name]);
        
        if (camera_flip_mir[i] == "r+") {
            cv::rotate(src, src, cv::ROTATE_90_CLOCKWISE);
        } else if (camera_flip_mir[i] == "r-") {
            cv::rotate(src, src, cv::ROTATE_90_COUNTERCLOCKWISE);
        } else if (camera_flip_mir[i] == "m") {
            cv::rotate(src, src, cv::ROTATE_180);
        }
        //display_mat(src, "project");
        //cv::imwrite(prms.name + "_undist.png", src);
        undist_dir_img[i] = src.clone();
    }
    //3.
    //TODO:add lum equalization and awb for four channel image
    std::cout  << argv[0] << " app start combine" << std::endl;
    car_img.copyTo(out_put_img(cv::Rect(xl, yt, car_img.cols, car_img.rows)));
    //4.1 out_put_img center copy 
    for (int i = 0; i < 4; ++i) {
        cv::Rect roi;
        bool is_cali_roi = false;
        if (std::string(camera_names[i]) == "front") {
            roi = cv::Rect(xl, 0, xr - xl, yt);
            //std::cout << "\nfront" << roi;
            undist_dir_img[i](roi).copyTo(out_put_img(roi));
        } else if (std::string(camera_names[i]) == "left") {
            roi = cv::Rect(0, yt, xl, yb - yt);
            //std::cout << "\nleft" << roi << out_put_img.size();
            undist_dir_img[i](roi).copyTo(out_put_img(roi));
        } else if (std::string(camera_names[i]) == "right") {
            roi = cv::Rect(0, yt, xl, yb - yt);
            //std::cout << "\nright" << roi << out_put_img.size();
            undist_dir_img[i](roi).copyTo(out_put_img(cv::Rect(xr, yt, total_w - xr, yb - yt)));
        } else if (std::string(camera_names[i]) == "back") {
            roi = cv::Rect(xl, 0, xr - xl, yt);
            //std::cout << "\nright" << roi << out_put_img.size();
            undist_dir_img[i](roi).copyTo(out_put_img(cv::Rect(xl, yb, xr - xl, yt)));
        } 
    }
    //4.2the four corner merge
    //w: 0 左下 1 右上 2 左上 3 左下
    //image: "front", "left", "back", "right"
    cv::Rect roi;
    //左上
    roi = cv::Rect(0, 0, xl, yt);
    merge_image(undist_dir_img[0](roi), undist_dir_img[1](roi), merge_weights_img[2], out_put_img(roi));
    //右上
    roi = cv::Rect(xr, 0, xl, yt);
    merge_image(undist_dir_img[0](roi), undist_dir_img[3](cv::Rect(0, 0, xl, yt)), merge_weights_img[1], out_put_img(cv::Rect(xr, 0, xl, yt)));
    //左下
    roi = cv::Rect(0, yb, xl, yt);
    merge_image(undist_dir_img[2](cv::Rect(0, 0, xl, yt)), undist_dir_img[1](roi), merge_weights_img[0], out_put_img(roi));
    //右下
    roi = cv::Rect(xr, 0, xl, yt);
    merge_image(undist_dir_img[2](roi), undist_dir_img[3](cv::Rect(0, yb, xl, yt)), merge_weights_img[3], out_put_img(cv::Rect(xr, yb, xl, yt)));

    cv::imwrite("ADAS_EYES_360_VIEW.png", out_put_img);
    
#ifdef DEBUG   
    cv::resize(out_put_img, out_put_img, cv::Size(out_put_img.size()/2)),
    display_mat(out_put_img, "out_put_img");
#endif

    std::cout  << argv[0] << " app finished" << std::endl;
}