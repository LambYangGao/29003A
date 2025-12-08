//
// Created by gdb on 2022/11/28.
//
#include "opencv2/opencv.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>

#include "JZSoTracker.h"


int getFilePaths(std::vector<std::string> &filepaths, cv::String filePath)
{
    filepaths.clear();
    std::cout << "Read files from: " << filePath << std::endl;
    std::vector<cv::String> fn;
    cv::glob(filePath, fn, false);

    if (fn.size() == 0)
    {
        std::cout << "file " << filePath << " not  exits" << std::endl;
        return -1;
    }
    //prepare pair for sort
    std::vector<std::pair<int, std::string>> v1;
    std::pair<int, std::string> p1;
    std::vector<cv::String >::iterator it_;
    for (it_ = fn.begin(); it_ != fn.end();	++it_)
    {
        //1.获取不带路径的文件名,1.txt
        std::string::size_type iPos = (*it_).find_last_of('/') + 1;
        std::string filename = (*it_).substr(iPos, (*it_).length() - iPos);
        //2.获取不带后缀的文件名,1
        std::string name = filename.substr(0, filename.rfind("."));
        //3.构建键和值的pair
        try {
            //防止文件夹中出现非整数的文件名导致的错误
            p1 = std::make_pair(stoi(name), (*it_).c_str());

        }catch(std::exception e)
        {
            std::cout << "Crushed -> " << e.what() << std::endl;
            //continue; 直接continue一样
            it_ = fn.erase(it_);
            //https://www.cnblogs.com/shiliuxinya/p/12219149.html
            it_--; //erase函数的返回的是指向被删除元素的下一个元素的迭代器，所以执行erase（）后要把迭代器减1，指向前面一个
        }
        v1.emplace_back(p1);
    }
    std::sort(v1.begin(), v1.end(), [](std::pair<int, std::string> a, std::pair<int, std::string> b) {return a.first < b.first; });
    std::vector<std::pair<int, std::string> >::iterator it;
    for (it = v1.begin(); it != v1.end(); ++it)
    {
        filepaths.emplace_back(it->second);
    }
    return 0;
}

int main(int argc, char * argv[])
{
    if(argc<3){
        std::cout<<"must have video path"<<std::endl;
        std::cout<<"demo mode(0:video,1:images) video_path|files path"<<std::endl;
        return -1;
    }

    int mode=std::stoi(argv[1]);
    JZSoTTracker tracker;
    tracker.disableLogInfo();
    tracker.setDisThresh(0.1);
    std::string file_path=argv[2];

    //mode=0: input type is video
    if(mode==0){
        cv::VideoCapture capture;
        capture.open(file_path);
        std::cout<<"opened successfully~"<<std::endl;
        if (!capture.isOpened())
        {
            std::cout << "capture device failed to open!" << std::endl;
            return -1;
        }

        cv::Rect box;
        cv::Mat frame;
        cv::Mat gray_frame;
        capture >> frame;

        box = cv::selectROI("tracker", frame);

        cv::cvtColor(frame,gray_frame,cv::COLOR_BGR2GRAY);
        tracker.init(box,gray_frame);

        int frameCount = 0;
        while (1)
        {
            capture >> frame;
            if (frame.empty())
                return -1;

            frameCount++;

            std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
            // tracking
            cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
            //box = tracker.update(gray_frame);
            int ret = tracker.update(gray_frame, box);

            std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
            std::chrono::milliseconds timeInterval = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
            std::cout << timeInterval.count() << " ms\n";


            // show the result
            std::stringstream buf;
            buf << frameCount;
            std::string num = buf.str();
            cv::putText(frame, num, cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 3);
            cv::rectangle(frame, box, cv::Scalar(0, 0, 255), 3);
            cv::namedWindow("Tracker", cv::WINDOW_NORMAL);
            cv::imshow("Tracker", frame);

            if ( cv::waitKey(1) == 27 )
                break;
        }
    }
    //文件命名为纯数字
    else if(mode==1){
        std::vector<std::string> filepaths;

        getFilePaths(filepaths,cv::String(file_path));
        for(int i=0;i<filepaths.size();i++){
            cv::Rect box;
            cv::Mat frame;
            cv::Mat gray_frame;
            frame=cv::imread(filepaths[i]);
            if(i==0){
                box = cv::selectROI("tracker", frame);
                cv::cvtColor(frame,gray_frame,cv::COLOR_BGR2GRAY);
                tracker.init(box,gray_frame);
            }


            std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
            // tracking
            cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
            box = tracker.update(gray_frame);

            std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
            std::chrono::milliseconds timeInterval = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
            std::cout << timeInterval.count() << " ms\n";


            // show the result
            std::stringstream buf;
            buf << i;
            std::string num = buf.str();
            cv::putText(frame, num, cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 3);
            cv::rectangle(frame, box, cv::Scalar(0, 0, 255), 3);
            cv::namedWindow("Tracker", cv::WINDOW_NORMAL);
            cv::imshow("Tracker", frame);
            cv::waitKey(50);
        }
    }

    return 0;
}
