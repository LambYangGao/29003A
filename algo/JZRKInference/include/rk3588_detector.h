//
// Created by gdb on 2023/4/3.
//

#ifndef  RK3588_DETECTOR_H
#define RK3588_DETECTOR_H

#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>

#include <chrono>
#include <stdlib.h>
#include "opencv2/opencv.hpp"

#include "common.h"
#include "base_detector.h"

class RK3588Detector:public BaseDetector{
public:
    RK3588Detector();

    virtual ~RK3588Detector();

    int32_t init(const ParamConfig &config);

    int32_t detect(const ModelInferenceInput &input_data,float confThesh,float nmsThresh,std::vector<BBoxInfo> &results);

    int32_t detect_cvmat(cv::Mat &image,float confThesh,float nmsThresh,std::vector<BBoxInfo> &results);
//backend engine
private:
    class AppImpl;
    AppImpl* RKNNImpl=nullptr;
};




#endif //RK3588_DETECTOR_H
