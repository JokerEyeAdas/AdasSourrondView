/***
 * function: 360 surrond view combine c++ demo
 * author: joker.mao
 * date: 2023/07/15
 * copyright: ADAS_EYES all right reserved
*/

#ifndef PRMS_H
#define PRMS_H

#include <iostream>
#include <map>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>


static const  char* camera_names[4] = {
    "front", "left", "back", "right"
};

static const  char* camera_flip_mir[4] = {
    "n", "r-", "m", "r+"
};
//单个格子10cm
//--------------------------------------------------------------------
//(shift_width, shift_height): how far away the birdview looks outside
//of the calibration pattern in horizontal and vertical directions
static const  int shift_w = 300;
static const  int shift_h = 300;

static const  int cali_map_w  = 600;
static const  int cali_map_h  = 1000;
//size of the gap between the calibration pattern and the car
//in horizontal and vertical directions
static const  int inn_shift_w = 20;
static const  int inn_shift_h = 50;

//total width/height of the stitched image
static const  int total_w = cali_map_w + 2 * shift_w;
static const  int total_h = cali_map_h + 2 * shift_h;

//four corners of the rectangular region occupied by the car
//top-left (x_left, y_top), bottom-right (x_right, y_bottom)
static const  int xl = shift_w + 180 + inn_shift_w;
static const  int xr = total_w - xl;
static const  int yt = shift_h + 200 + inn_shift_h;
static const  int yb = total_h - yt;
//--------------------------------------------------------------------

static std::map<std::string, cv::Size> project_shapes = {
    {"front",  cv::Size(total_w, yt)},
    {"back",   cv::Size(total_w, yt)},
    {"left",   cv::Size(total_h, xl)},
    {"right",  cv::Size(total_h, xl)},
};

//pixel locations of the four points to be chosen.
//you must click these pixels in the same order when running
//the get_projection_map.py script
static std::map<std::string, std::vector<cv::Point2f>> project_keypoints = {
    {"front", {cv::Point2f(shift_w + 120, shift_h),
              cv::Point2f(shift_w + 480, shift_h),
              cv::Point2f(shift_w + 120, shift_h + 160),
              cv::Point2f(shift_w + 480, shift_h + 160)}},

    {"back", {cv::Point2f(shift_w + 120, shift_h),
              cv::Point2f(shift_w + 480, shift_h),
              cv::Point2f(shift_w + 120, shift_h + 160),
              cv::Point2f(shift_w + 480, shift_h + 160)}},

    {"left", {cv::Point2f(shift_h + 280, shift_w),
              cv::Point2f(shift_h + 840, shift_w),
              cv::Point2f(shift_h + 280, shift_w + 160),
              cv::Point2f(shift_h + 840, shift_w + 160)}},

    {"right", {cv::Point2f(shift_h + 160, shift_w),
              cv::Point2f(shift_h + 720, shift_w),
              cv::Point2f(shift_h + 160, shift_w + 160),
              cv::Point2f(shift_h + 720, shift_w + 160)}}
};

#endif