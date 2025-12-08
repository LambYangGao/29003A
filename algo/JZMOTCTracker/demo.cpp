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
#include "JZMotCTracker.h"
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
struct BBoxInfo
{
    float score;
    float xmin;
    float ymin;
    float xmax;
    float ymax;
    float area;
    int32_t label;
    int32_t track_id;
};


int main(int argc, char** argv) {

    std::vector<BBoxInfo> results;
	MotCTracker motTracker;
	motTracker.init(20,150,10,0.4);
	std::vector<bbox_tr> dets;
	for(int i=0;i<results.size();i++)
	{
		bbox_tr obj;
		obj.x=results[i].xmin;
		obj.y=result[i].ymin;
		obj.w=results[i].xmax-results[i].xmin;
		obj.h=results[i].ymax-results[i].ymin;
		obj.class_id=results[i].label;
		obj.prob=results[i].score;
		dets.push_back(prob);
	}
	motTracker.track(dets);
	
	//跟踪结果可以直接获取 二维数组，第一维表示类别，第二维表示某一个类别的某个目标
	//eg：motTracker.m_vhistoryTracks[i][j],表示第i类第j个目标；
	std::cout<<motTracker.m_vhistoryTracks.size()<<std::endl;;

    return 0;
}

