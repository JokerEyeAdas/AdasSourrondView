/***
 * function: 360 surrond view combine c++ demo
 * author: joker.mao
 * date: 2023/07/15
 * copyright: ADAS_EYES all right reserved
*/


#ifndef COM_H
#define COM_H

#include "prms.hpp"

struct camera_prms
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

void display_mat(cv::Mat& img, std::string name);
bool read_prms(const std::string& path, camera_prms& prms);
bool save_prms(const std::string& path, camera_prms& prms);
void undist_by_remap(const cv::Mat& src, cv::Mat& dst, const camera_prms& prms);

#endif