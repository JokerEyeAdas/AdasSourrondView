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


bool read_prms(const std::string& path, camera_prms& prms)
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

bool save_prms(const std::string& path, camera_prms& prms)
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

void undist_by_remap(const cv::Mat& src, cv::Mat& dst, const camera_prms& prms)
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
