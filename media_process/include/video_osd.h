#ifndef VIDEO_OSD_H
#define VIDEO_OSD_H

#include <stdint.h>
#include <string>

#include "rk_type.h"
#include "protocol_common.h"
#include "protocol_board_abc.h"
#include <sys/stat.h>
#include <mutex>
#include <condition_variable>
class VIDEO_OSD
{
	//都用1920*1080处理，处理完再送入vpss拉伸
public:
	VIDEO_OSD();
	~VIDEO_OSD();

	bool m_bmodVal = false;
	std::mutex mtx;
	std::condition_variable m_cond;
	//参数结构体
	struct VideoOsdCfg
	{
		RK_BOOL bThreadExit = RK_FALSE;//线程退出标志
		
    	// 主视图选择（1 可见光 2 红外）- 仅用于OSD叠加决策
    	RK_U32 u32MainView = 1;//当前主画面标记

    	// 两路视频的VPSS和VENC配置
    	RK_U32 u32VpssGroup[2] = {0, 1};     // 两路VPSS组号 [可见光, 红外]
    	RK_U32 u32VpssChn[2] = {0, 0};       // 两路VPSS通道号
    	RK_U32 u32VencChn[2] = {0, 1};       // 两路VENC通道号
		
    	// 两路图像尺寸
    	RK_U32 u32Width[2] = {1920, 640};    // [可见光, 红外]
    	RK_U32 u32Height[2] = {1080, 512};
    	RK_U32 u32OriWidth[2] = {1920, 640}; // 原始图像尺寸
    	RK_U32 u32OriHeight[2] = {1080, 512};
		
    	RK_S32 s32TimeOutMs = 200;//最大延时

		/*---------------------------------------------------------------------------------*/
		RK_BOOL bOsdFrame = RK_FALSE;//osd画中画开关
		RK_U32 u32FrameGroup = 1;//待画中画 vpss 组号 默认
		RK_U32 u32FrameChn = 0;//待画中画 vpss 通道号 默认
		RK_U32 u32OsdFrameX = 1350;//画中画坐标X
		RK_U32 u32OsdFrameY = 50;//画中画坐标Y
		RK_U32 u32OsdFrameLoc = 2;//画中画位置 1左上 2右上 3右下 4左下

		// OSD开关
		RK_BOOL bOsdCross = RK_TRUE;//osd十字开关
		RK_BOOL bOsdRect = RK_TRUE;//osd目标框开关
		RK_BOOL bOsdText = RK_TRUE;//osd文字开关
		RK_BOOL bOsdScale = RK_TRUE;//osd比例尺开关
		RK_U32 u32OsdLevel = 1;//osd等级 1，2，3
		RK_U32 u32SendVpss = 2;//发送的vpss组号 默认
		RK_U32 u32LineColorLevel = 1;//1 - 白色，2 - 黑色，3 - 红色，4 - 绿色
		RK_U32 u32LineColor = 0x00FFFFFF;//00 b g r

    //设置主画面（只改变主视图标记，不再切换通道）
    void setMainView(int num)
    {
        u32MainView = num;
        // 可以根据主视图调整OSD等级
        if (num == 1) {  // 可见光
            setOsdLevel(2);
        } else if (num == 2) {  // 红外
            setOsdLevel(2);
        }
    }
		//设置osd等级
		void setOsdLevel(int level)
		{
			u32OsdLevel = level;
			switch (u32OsdLevel)
			{
			case 0:
				break;
			case 1://全开
				bOsdCross = RK_TRUE;
				bOsdRect = RK_TRUE;
				bOsdText = RK_TRUE;
				bOsdScale = RK_TRUE;
				break;
			case 2://文字关 框开 十字开
				bOsdCross = RK_TRUE;
				bOsdRect = RK_TRUE;
				bOsdText = RK_FALSE;
				bOsdScale = RK_FALSE;
				break;
			case 3://全关
				bOsdCross = RK_FALSE;
				bOsdRect = RK_FALSE;
				bOsdText = RK_FALSE;
				bOsdScale = RK_FALSE;
				break;
			default:
				break;
			}
		}
		//设置瞄准线颜色
		void setLineColorLevel(int level)
		{
			u32LineColorLevel = level;
			switch (u32LineColorLevel)
			{
			case 0:
				break;
			case 1://白
				u32LineColor = 0x00FFFFFF;
				break;
			case 2://黑
				u32LineColor = 0x00000000;
				break;
			case 3://红
				u32LineColor = 0x000000FF;
				break;
			case 4://绿
				u32LineColor = 0x0000FF00;
				break;
			default:
				break;
			}
		}

		//设置画中画位置(顺时针旋转)
		void setOsdFrameLoc()
		{
			u32OsdFrameLoc++;
			if (u32OsdFrameLoc > 4)
			{
				u32OsdFrameLoc = 1;
			}
			switch (u32OsdFrameLoc)
			{
			case 1:
				u32OsdFrameX = 90;//画中画坐标X
				u32OsdFrameY = 50;//画中画坐标Y
				break;
			case 2:
				u32OsdFrameX = 1350;//画中画坐标X
				u32OsdFrameY = 50;//画中画坐标Y
				break;
			case 3:
				u32OsdFrameX = 90;//画中画坐标X
				u32OsdFrameY = 640;//画中画坐标Y
				break;
			case 4:
				u32OsdFrameX = 1350;//画中画坐标X
				u32OsdFrameY = 640;//画中画坐标Y
				break;
			default:
				break;
			}
		}
	};

	//配置参数 
	VideoOsdCfg* cfg = nullptr;

	void init();
	void run();


private:
	void processVideoStream(int streamIdx);  // 处理视频流
	
};

#endif //VIDEO_OSD_H