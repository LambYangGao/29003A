#ifndef TARGET_ENGINE_H
#define TARGET_ENGINE_H

#include <stdint.h>
#include <string>
#include "JZSoTracker.h"
#include "targetlocation.h"
#include "rk_type.h"
#include "rk_comm_video.h"
#include "rk_mpi_vpss.h"
#include "rk_debug.h"
#include "ConfigManager.h"

#include <mutex>


extern ConfigManager* g_configManager;//系统配置文件

#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐
	//目标跟踪结果--修改同步到RK3588ABC_MSG.H
struct TargetTrackRes
{
	uint16_t trackMode = 0;//0关 1开
	int16_t trackState = 0;//0正常 -1不正常 -2丢失
	cv::Rect_<float> predBox;//跟踪预测框

	int16_t target_x = 0;//水平坐标差
	int16_t target_y = 0;//垂直坐标差
	float target_fw = 0.0;//水平脱靶量（引入偏差）
	float target_fy = 0.0;//垂直脱靶量（引入偏差）
	
	float target_distance = 0;
	double target_jd = 0;//目标经度
	double target_wd = 0;//目标纬度
	double target_gd = 0;//目标高度

	double target_ve;//目标东向速度
	double target_vn;//目标北向速度
	double target_vu;//目标天向速度
};

#pragma pack(pop)
class TARGET_ENGINE
{
public:
	TARGET_ENGINE();
	~TARGET_ENGINE();

	//参数结构体
	struct TargetEngineCfg
	{
		RK_BOOL bThreadExit = RK_FALSE;  //线程退出标志
		RK_U32 u32MainView = 1;               //当前主画面 //实时维护	 1: 电视 2:红外
		RK_U32 u32VpssGroup = 0;             //vpss组号 默认
		RK_U32 u32VpssChn = 2;                //vpss通道号 根据主画面选择
		RK_U32 u32OriWidth = 1920;          //原始图像宽度
		RK_U32 u32OriHeight = 1080;         //原始图像高度
		RK_U32 u32Width = 1920;//输入图像宽度 = 原始图像宽度
		RK_U32 u32Height = 1080;//输入图像高度 = 原始图像高度
		RK_S32 s32TimeOutMs = 200;//最大延时
		/*---------------------------------------------------------------------------------*/
		RK_BOOL b_sot_on = RK_FALSE;   //目标跟踪开关 //实时维护
		RK_BOOL b_location_on = RK_FALSE; //目标主动定位开关 目标定位打开测距//实时维护
		RK_BOOL b_velocity_on = RK_FALSE; //速度测量开关 测距后开启速度测量 1s后速度值可信//实时维护
		RK_U32 u32BoxSize = 3;//波门大小 1小 2中 3大 默认中
		RK_U32 trackMemoryCountMax = 1500;//记忆跟踪最大计数

		cv::Rect_<float> initBox = cv::Rect_<float>(905, 515, 90, 50);           //波门 //实时维护
		cv::Rect_<float> initBox_extern= cv::Rect_<float>(0, 0, 0, 0); //外部检测框或显控下发的跟踪初始框
		const cv::Rect_<float> VI_L_initBox = cv::Rect_<float>(840, 450, 240, 180);//960  540
		const  cv::Rect_<float> VI_M_initBox = cv::Rect_<float>(925, 515, 120, 90);
		const cv::Rect_<float> VI_S_initBox = cv::Rect_<float>(940, 527, 40, 26);
		const cv::Rect_<float> IRz_L_initBox = cv::Rect_<float>(535, 428, 210, 168);//640 512
		const  cv::Rect_<float> IRz_M_initBox = cv::Rect_<float>(610, 488, 60, 48);
		const  cv::Rect_<float> IRz_S_initBox = cv::Rect_<float>(625, 500, 30, 24);
		const  cv::Rect_<float> IRd_L_initBox = cv::Rect_<float>(267, 214, 106, 84);//320 256
		const  cv::Rect_<float> IRd_M_initBox = cv::Rect_<float>(305, 244, 30, 24);
		const  cv::Rect_<float> IRd_S_initBox = cv::Rect_<float>(313, 250, 14, 12);

		RK_FLOAT dissapearThesh = 0.1;//目标检测置信度

		bool m_is_external_init = false;
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
				u32VpssChn = 2;
				u32OriWidth = 1920;
				u32OriHeight = 1080;
				u32Width = 1920;
				u32Height = 1080;
				if (u32BoxSize == 1)
				{
					initBox = VI_S_initBox;
				}
				else if (u32BoxSize == 2)
				{
					initBox = VI_M_initBox;
				}
				else
				{
					initBox = VI_L_initBox;
				}
				break;
			case 2://红外
				u32VpssGroup = 1;
				u32VpssChn = 2;
				u32OriWidth = 640;
				u32OriHeight = 512;
				u32Width = 640;
				u32Height = 512;
				if (u32BoxSize == 1)
				{
					initBox = IRd_S_initBox;
				}
				else if (u32BoxSize == 2)
				{
					initBox = IRd_M_initBox;
				}
				else
				{
					initBox = IRd_L_initBox;
				}
				break;
			default:
				break;
			}
		}
	};

	//配置参数 
	TargetEngineCfg* cfg = nullptr;
	//跟踪结果
	TargetTrackRes* trackRes = nullptr;

	void init();
	void run();

	//目标跟踪关闭
	void trackOff();
	void trackOn(cv::Rect &outInitBox);

	//目标跟踪结果信息同步
	void trackInfoSycToB();
private:
	std::mutex mtx;
	bool m_is_external_init = false;

	//目标跟踪
	JZSoTTracker* tracker=nullptr;
	//目标定位
	TargetLoclization* targetLoc = nullptr;


};

#endif //TARGET_ENGINE