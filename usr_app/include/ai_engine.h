#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include <stdint.h>
#include <string>
#include "rk3588_detector.h"//目标检测
#include "rk3588classifier.h"//属性识别
#include "JZMotCTracker.h"  //多目标跟踪

#include "protocol_board_abc.h"
#include "rk_type.h"
#include "rk_mpi_vpss.h"
#include "rk_debug.h"

#include <mutex>
#include <unistd.h>

extern BOARDABC_MSG* boardabcMsg;;//RK网口通信

class AI_ENGINE
{
public:
	AI_ENGINE(void);
	~AI_ENGINE(void);

	//参数结构体
	struct AiEngineCfg
	{
		RK_BOOL bThreadExit = RK_FALSE;//线程退出标志
		RK_U32 u32MainView = 1;//当前主画面 //实时维护
		RK_U32 u32VpssGroup = 0;//vpss组号
		RK_U32 u32VpssChn = 1;   //vpss通道号
		RK_U32 u32PicWidth = 640;  //模型输入图像宽度
		RK_U32 u32PicHeight = 512; //模型输入图像高度
		RK_U32 u32OriWidth = 1920;//原始图像宽度   与当前主路视频大小有关系，若为电视则为1920x1080，红外则是640x512
		RK_U32 u32OriHeight = 1080;//原始图像高度
		RK_S32 s32TimeOutMs = 200;//最大延时

		/*---------------------------------------------------------------------------------*/

		RK_BOOL b_detect_on = RK_FALSE;//目标检测开关 检测开启时，多目标跟踪自动开启 //实时维护
		RK_BOOL b_mot_on = RK_FALSE;//多目标跟踪开关 //实时维护 

		RK_U32 detectLevel = 1;//目标检测挡位 1-9 ，对应阈值0.1 - 0.9 //实时维护
		RK_FLOAT confThesh = 0.3;//目标检测置信度 
		RK_FLOAT nmsThresh = 0.45;//目标检测非极大值抑制

		RK_U32 mot_class_num = 6;//多目标跟踪类别
		RK_U32 mot_queue_num = 100;//多目标跟踪轨迹长度
		RK_U32 mot_loss_num = 6;//多目标跟踪丢失次数
		RK_FLOAT mot_iou_thresh = 0.1;//多目标跟踪IOU

		std::string detect_model_path = "./resource/models/yolov8_v3.1.rknn";//目标检测模型地址

		//设置主画面
		void setMainView(int num)
		{
			u32MainView = num;
			switch (u32MainView)
			{
			case 0:
				break;
			case 1://电视
				u32VpssGroup = 0;
				u32VpssChn = 1;
				u32OriWidth = 1920;
				u32OriHeight = 1080;
				break;
			case 2://中波红外
				u32VpssGroup = 1;
				u32VpssChn = 1;
				u32OriWidth = 640;
				u32OriHeight = 512;
				break;
			default:
				break;
			}
		}
		//目标检测阈值加档
		void detectTheshUP(void)
		{
			detectLevel ++;
			if (detectLevel >= 9)
			{
				detectLevel = 9;
			}
			confThesh = (RK_FLOAT)detectLevel / 10;
		}
		//目标检测阈值减档
		void detectTheshDown(void)
		{
			detectLevel--;
			if (detectLevel <= 1)
			{
				detectLevel = 1;
			}
			confThesh = (RK_FLOAT)detectLevel / 10;
		}
		//多目标提示开关
		void motPromptOn(bool flag)
		{
			if (flag)
			{
				b_detect_on = RK_TRUE;
				b_mot_on = RK_TRUE;
			}
			else
			{
				b_detect_on = RK_FALSE;
				b_mot_on = RK_FALSE;
			}
		}
	};

	//配置参数 
	AiEngineCfg* cfg = nullptr;
	//目标结果
	std::vector<std::vector<ObjectInfo>> trackObjs;                        //类别/数量 <对象> 船，浮标，人，车，建筑，坦克
	std::vector<TargetDetectRes_A2B_MSG> targetDetectRes_Msg;    //目标检测结果传输到B板进行OSD叠加  在A板也可进行测试



	void init(void);
	void run(void);

private:
	std::mutex mtx;
	//目标检测
	ParamConfig detectParams;
	RK3588Detector* detector = nullptr;
	//多目标跟踪
	MotCTracker* motTracker = nullptr;
	//目标结果
	std::vector<BBoxInfo> results;//模型推理结果;

	void letterbox(const cv::Mat &originImage, const cv::Size& inpSize, cv::Mat& paddingImage);
	void mapCoordToOriginal(cv::Rect& bbox, int cur_height, int cur_width, int ori_height, int ori_width);
};

#endif //AI_ENGINE_H