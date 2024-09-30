/***
 * function: 360 surrond view combine c++ demo
 * author: joker.mao
 * date: 2023/07/15
 * copyright: ADAS_EYES all right reserved
*/


#include "common.h"

void display_mat(cv::Mat& img, std::string name)
{
    cv::imshow(name, img);
    cv::waitKey();
}

//read cali prms
bool read_prms(const std::string& path, CameraPrms& prms)
{
    cv::FileStorage fs(path, cv::FileStorage::READ);

    if (!fs.isOpened()) {
        throw std::string("error open file");
        return false;
    }
    
    prms.camera_matrix  = fs["camera_matrix"].mat();
    prms.dist_coff      = fs["dist_coeffs"].mat();
    prms.project_matrix = fs["project_matrix"].mat();
    prms.shift_xy       = fs["shift_xy"].mat();
    prms.scale_xy       = fs["scale_xy"].mat();
    auto size_          = fs["resolution"].mat();
    prms.size           = cv::Size(size_.at<int>(0), size_.at<int>(1));

    fs.release();
    
    return true;
}
//save cali prms
bool save_prms(const std::string& path, CameraPrms& prms)
{
    cv::FileStorage fs(path, cv::FileStorage::WRITE);

    if (!fs.isOpened()) {
        throw std::string("error open file");
        return false;
    }
    
    if (!prms.project_matrix.empty())
        fs << "project_matrix" << prms.project_matrix;

    fs.release();
    
    return true;
}

//undist image by remap
void undist_by_remap(const cv::Mat& src, cv::Mat& dst, const CameraPrms& prms)
{
    //get new camera matrix
    cv::Mat new_camera_matrix = prms.camera_matrix.clone();
    double* matrix_data = (double *)new_camera_matrix.data;

    const auto scale = (const float *)(prms.scale_xy.data);
    const auto shift = (const float * )(prms.shift_xy.data);

    if (!matrix_data || !scale || !shift) {
        return;
    }

    matrix_data[0]         *=  (double)scale[0];
    matrix_data[3 * 1 + 1] *=  (double)scale[1];
    matrix_data[2]         +=  (double)shift[0];
    matrix_data[1 * 3 + 2] +=  (double)shift[1];
    //std::cout << new_camera_matrix;
    //undistort
    cv::Mat map1, map2;
    cv::fisheye::initUndistortRectifyMap(prms.camera_matrix, prms.dist_coff, cv::Mat(), new_camera_matrix, prms.size , CV_16SC2, map1, map2);
    
    cv::remap(src, dst, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);   
}

//merge image by weights
void merge_image(cv::Mat src1, cv::Mat src2, cv::Mat w, cv::Mat out)
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
            o[0] = clip<uint8_t>(p1[0] * weights[p_index] + p2[0] * (1 - weights[p_index]), 255);
            o[1] = clip<uint8_t>(p1[1] * weights[p_index] + p2[1] * (1 - weights[p_index]), 255);
            o[2] = clip<uint8_t>(p1[2] * weights[p_index] + p2[2] * (1 - weights[p_index]), 255);
            p1 += 3;
            p2 += 3;
            o  += 3;
            ++p_index;
        }
    }
}

//r g b channel statics
void rgb_info_statics(cv::Mat& src, BgrSts& sts)
{
    int nums = src.rows * src.cols;

    sts.b = sts.r = sts.g = 0;

    for (int h = 0; h < src.rows; ++h) {
        uchar* uc_pixel = src.data + h * src.step;
        for (int w = 0; w < src.cols; ++w) {
            sts.b += uc_pixel[0];
            sts.g += uc_pixel[1];
            sts.r += uc_pixel[2];
            uc_pixel += 3;
        }
    }

    sts.b /= nums;
    sts.r /= nums;
    sts.g /= nums;
}

//r g b digtial gain
void rgb_dgain(cv::Mat& src, float r_gain, float g_gain, float b_gain)
{
    if (src.empty()) {
        return;
    }
    for (int h = 0; h < src.rows; ++h) {
        uchar* uc_pixel = src.data + h * src.step;
        for (int w = 0; w < src.cols; ++w) {
            uc_pixel[0]  = clip<uint8_t>(uc_pixel[0] * b_gain, 255);
            uc_pixel[1]  = clip<uint8_t>(uc_pixel[1] * g_gain, 255);
            uc_pixel[2]  = clip<uint8_t>(uc_pixel[2] * r_gain, 255);
            uc_pixel += 3;
        }
    }
}

//gray world awb amd lum banlance for four channeal images
void awb_and_lum_banlance(std::vector<cv::Mat*> srcs)
{
    BgrSts sts[4];
    int    gray[4] = {0, 0, 0, 0};
    float    gray_ave = 0;

    if (srcs.size() != 4) {
        return;
    }

    for (int i = 0; i < 4; ++i) {
        if (srcs[i] == nullptr) {
            return;
        }
        rgb_info_statics(*srcs[i], sts[i]);
        gray[i] = sts[i].r * 20 + sts[i].g * 60 + sts[i].b;
        gray_ave += gray[i];
    }
    
    gray_ave /= 4;

    for (int i = 0; i < 4; ++i) {
        float lum_gain = gray_ave / gray[i];
        float r_gain = sts[i].g * lum_gain / sts[i].r;
        float g_gain = lum_gain;
        float b_gain = sts[i].g * lum_gain / sts[i].b;
        //std::cout << "gains : " << r_gain << " | " << g_gain << " | " << b_gain << "\r\n";
        rgb_dgain(*srcs[i], r_gain, g_gain, b_gain);
    }
}