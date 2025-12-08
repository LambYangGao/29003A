#include <iostream>
#include <vector>
#include <chrono>


#include<stdlib.h>   
#include<stdio.h>    
#include<sys/sysinfo.h>   
#include<unistd.h>  
#define __USE_GNU   
#include<sched.h>   
#include<ctype.h>   
#include<string.h>  

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/prctl.h>

#include "opencv2/opencv.hpp"

//#include "opencv2/videoio.hpp"

#include "rk3588_detector.h"
#include "rk3588classifier.h"

//#include <stdlib.h>
using namespace std;
using namespace chrono;

const std::vector<std::string> class_names = { "boat","banner","person","vechile","building"};


void set_cpu_core(int cpuId)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpuId, &mask);
    //线程级别的绑定
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
        printf("set thread affinity failed");
    printf("Bind process to CPU %d\n", cpuId);
}

int main(int argc, char** argv) {

    if (argc != 5) {
        std::cout << "need 4 parameters! please check!" << std::endl;
        std::cout << "usage  ./test test_mode(0|1) model_path anchor_txt_file image_path" << std::endl;
    }
    ParamConfig params;
    std::string rknn_model_path = argv[2];
    std::string image_path = "test.jpg";
    std::string save_path = "res.jpg";
    std::string anchor_file = argv[3];

    int test_mode = std::stoi(argv[1]);
    image_path = argv[4];

    set_cpu_core(0);

    if (test_mode == 0) {
        RK3588Detector* detector = new RK3588Detector();
        params.model_path = rknn_model_path;
        params.anchor_path = anchor_file;
        params.algo_type = "yolov8";
        params.platform = DEVICE_PLATFORM::RK3588;
        params.device_id = 0;
        detector->init(params);

        std::cout << "start infer: " << std::endl;
        std::vector<BBoxInfo> results;
        cv::Mat image = cv::imread(image_path);
        std::cout << "read image finished" << std::endl;
        double time_sum = 0.f;
        for (int i = 0; i < 100; i++) {
            results.clear();
            auto start = system_clock::now();
            detector->detect_cvmat(image, 0.1, 0.45, results);
            auto end = system_clock::now();
            auto duration = duration_cast<milliseconds>(end - start);
            std::cout << "cost time: " << double(duration.count()) << " ms" <<"   target_num: "<< results.size() << endl;
            time_sum += double(duration.count());

        }
        std::cout << "average time: " << time_sum / 100 << " ms" << endl;
        for (int i = 0; i < results.size(); i++) {
            cv::Point pt1 = cv::Point(results[i].xmin, results[i].ymin);
            cv::Point pt2 = cv::Point(results[i].xmax, results[i].ymax);
            int label_id = results[i].label;
            cv::rectangle(image, pt1, pt2, cv::Scalar(0, 255, 255), 2);
            cv::putText(image, class_names[label_id], cv::Point(pt1.x, pt1.y - 10), cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0, 255, 255));
        }
        cv::imwrite(save_path, image);
        delete detector;
        detector = NULL;
    }
     else{
            RK3588Classifer* classsifier = new RK3588Classifer();
            params.model_path = rknn_model_path;
            params.anchor_path = anchor_file;
            params.algo_type = "yolov8";
            params.platform = DEVICE_PLATFORM::RK3588;
            classsifier->init(params);

            std::cout << "start infer: " << std::endl;
            
            cv::Mat image = cv::imread(image_path);
            cv::resize(image, image, cv::Size(128, 128));
            std::cout << "read image finished" << std::endl;
            double time_sum = 0.f;
            for (int i = 0; i < 10000; i++) {
                ClsOnfo res;
                auto start = system_clock::now();
                classsifier->apply(image, res);
                auto end = system_clock::now();
                auto duration = duration_cast<milliseconds>(end - start);
                std::cout << "cost time: " << double(duration.count()) << " ms" << endl;
                time_sum += double(duration.count());

            }
            std::cout << "average time: " << time_sum / 10000.0 << " ms" << endl;
     }



    return 0;
}

