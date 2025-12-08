#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>

#include <chrono>
#include <stdlib.h>
#include "opencv2/opencv.hpp"

#include "common.h"

class RK3588Classifer
{
public:
    RK3588Classifer();

	~RK3588Classifer();

    int32_t init(const ParamConfig& config);

    int32_t apply(cv::Mat& input_data, BBoxInfo &obj);

private:
    class AppImpl;
    AppImpl* RKNNImpl = nullptr;

};