//
// Created by gdb on 2023/3/28.
//

#ifndef JZENGINEV1_0_BASE_DETECTOR_H
#define JZENGINEV1_0_BASE_DETECTOR_H
#include <stdio.h>
#include "common.h"




class BaseDetector
{
public:
    BaseDetector(){};

    virtual ~BaseDetector(){};

    virtual int32_t init(const ParamConfig &config)=0;

    virtual int32_t detect(const ModelInferenceInput &input_data,float confThesh,float nmsThresh,std::vector<BBoxInfo> &results)=0;
};


#endif //JZENGINEV1_0_BASE_DETECTOR_H
