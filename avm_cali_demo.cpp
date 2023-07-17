/***
 * function: 360 surrond view combine c++ demo
 * author: joker.mao
 * date: 2023/07/15
 * copyright: ADAS_EYES all right reserved
*/

#include <iostream>
#include <map>
#include <vector>
#include "common.h"

struct mouse_prms {
    cv::Mat* mat;
    std::vector<cv::Point2f> star_points;
    std::string win_name;

    mouse_prms() {
        mat = nullptr;
        star_points.clear();
        win_name = "front";
    }
};

void on_mouse(int event, int x, int y, int flags, void* param)
{
    mouse_prms* prms = (mouse_prms*)param;
    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:
        std::cout << x << y << std::endl;
        cv::putText(*prms->mat, "[" + std::to_string(x) + "," + std::to_string(y) + "]", cv::Point(x, y), 1, 1.0, cv::Scalar(0, 0, 255));
        cv::imshow(prms->win_name, *prms->mat);
        prms->star_points.push_back(cv::Point(x, y));
        if (prms->star_points.size() == 4) {
            cv::destroyWindow(prms->win_name);
        }
        break;
    
    default:
        break;
    }
}


int main(int argc, char** argv)
{
    std::cout  << argv[0] << " app start running..." << std::endl;
    
    for (int i = 0; i < 4; ++i) {
        CameraPrms prms;
        prms.name = camera_names[i];

        cv::Mat src = cv::imread("../../images/" + prms.name + ".png");
        
        read_prms("../../yaml/" + prms.name + ".yaml", prms);
        undist_by_remap(src, src, prms);
        //if has not calibration，then we will cali it
        if (prms.project_matrix.empty()) 
        {
            //cali
            mouse_prms m_prms;
            cv::Mat mat_display = src.clone();
            cv::imshow(prms.name, mat_display);
            m_prms.mat = &mat_display;
            m_prms.win_name = prms.name;
            //get trans points
            cv::setMouseCallback(prms.name, on_mouse, &m_prms);
            cv::waitKey();

            prms.project_matrix =  cv::getPerspectiveTransform(m_prms.star_points, project_keypoints[prms.name]);
            cv::warpPerspective(src, src, prms.project_matrix, project_shapes[prms.name]);
            save_prms("../../yaml/project_" + prms.name + ".yaml", prms);
            cv::imwrite(prms.name + "_cali.png", mat_display);
        }

        cv::warpPerspective(src, src, prms.project_matrix, project_shapes[prms.name]);
        display_mat(src, "project");
    }
    std::cout << "cali finished\r\n";
    return 0;
}