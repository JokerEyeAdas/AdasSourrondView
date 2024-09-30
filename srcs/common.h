/***
 * function: 360 surrond view combine c++ demo
 * author: joker.mao
 * date: 2023/07/15
 * copyright: ADAS_EYES all right reserved
*/


#ifndef COM_H
#define COM_H

#include "prms.hpp"

struct CameraPrms
{
    std::string name;
    cv::Mat dist_coff;
    cv::Mat camera_matrix;
    cv::Mat project_matrix;
    cv::Mat trans_matrix;
    cv::Size size;

    cv::Mat scale_xy;
    cv::Mat shift_xy;
};

struct BgrSts {
    int b;
    int g;
    int r;

    BgrSts() {
        b = g = r = 0;
    }
};

template<typename _T>
static inline _T clip(float data, int max)
{
    if (data > max)
        return max;
    return (_T)data;
}

void display_mat(cv::Mat& img, std::string name);
bool read_prms(const std::string& path, CameraPrms& prms);
bool save_prms(const std::string& path, CameraPrms& prms);
void undist_by_remap(const cv::Mat& src, cv::Mat& dst, const CameraPrms& prms);

void merge_image(cv::Mat src1, cv::Mat src2, cv::Mat w, cv::Mat out);
void awb_and_lum_banlance(std::vector<cv::Mat*> srcs);
#endif