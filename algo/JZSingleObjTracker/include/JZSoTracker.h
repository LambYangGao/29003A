//
// Created by gdb on 2022/11/28.
// update on the 2023.09.09
//

#ifndef FDSST_TRACKER_JZFDSSTTRACKER_H
#define FDSST_TRACKER_JZFDSSTTRACKER_H

#include <opencv2/opencv.hpp>
#include <memory>
#include <algorithm>
class JZSoTTracker {
public:
    JZSoTTracker(const std::string &algo_method="fdsst");
    ~JZSoTTracker();

    int init(const cv::Rect_<float>&roi, cv::Mat &image);

    void enableLogInfo();

    void disableLogInfo();

    void setDisThresh(float disThresh=0.01);

    //0:normal -1: abnormal -2： dissappear
    int update(cv::Mat &image, cv::Rect_<float>&pred_box);

    //old track !
    //cv::Rect update(cv::Mat &image);
private:
    class TrackerImpl;
    TrackerImpl* trackerImpl=nullptr;
};


#endif //FDSST_TRACKER_JZFDSSTTRACKER_H
