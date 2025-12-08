#ifndef __INFER_COMMON_H__
#define __INFER_COMMON_H__

#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
/*
    tensor data order
    0:nchw 1:nhwc
*/
struct TensorShape{
    int32_t fmt=0; 
    int32_t order=0;   
    int32_t num;
    int32_t channel;
    int32_t height;
    int32_t width;
};

struct BBoxInfo
{
    float score;
    float xmin;
    float ymin;
    float xmax;
    float ymax;
    float area;
    int32_t label;

    int32_t x; //左上角X
    int32_t y; //左上角
    int32_t w;
    int32_t h;

    int32_t attrib_id=-1;
    float attrib_prob;
};

typedef enum IMAGE_TYPE{
    BGR_PACKAGE=0,
    BGR_PLANAR,
    RGB_PACKAGE,
    RGB_PLANAR,
    YUV_420SP,
    YUV_422SP,
}IMAGE_FORMAT_S;

enum DEVICE_PLATFORM{
    RK3399PRO=0,
    RK3588,
    HI3559A,
    HI3519A,
};

struct ModelInferenceInput
{
    unsigned char *img_data;
    IMAGE_FORMAT_S dataType;
    int32_t data_format; //BGR=0,RGB=1,NV21=2,NV12=3,S16=4,BGR_PLANAR=5
	int32_t img_w;
    int32_t img_w_stride;
	int32_t img_h;
    int32_t img_h_stride;
    int32_t channel;
};

struct ParamConfig
{
    int32_t device_id;
    std::string model_path;
    std::string anchor_path;
    std::string algo_type;  //yolov5��yolov8
    bool with_softmax=false;
    DEVICE_PLATFORM platform;
};

enum EAIDETLabel
{
    BOAT = 0,
    BANNER=1,
    PERSON =2 ,
    VEHICLE =3 ,
    BUILDING=4,
    TANK=5
};

enum EAICLSLabel
{
    VEHICLE_M = 0,
    VEHICLE_J = 1,
    BOAT_M = 2,
    BOAT_J = 3
};
#endif