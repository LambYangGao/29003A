#include <memory>
#include <thread>
#include <locale>
#include <rk_type.h>
#include <fan_tsensor.h>
#include "vpss_engine.h"
#include "video_osd.h"
#include "ConfigManager.h"
#include "protocol_vi.h"
#include "protocol_ir.h"
#include "protocol_picxk.h"
#include "xkMsg.h"
#include "protocol_common.h"
#include "protocol_jgzm.h"
#include "protocol_board_abc.h"
#include "serialport.h"
#include "target_engine.h"
#include "ai_engine.h"
#include "protocol_power.h"
#include "protocol_env.h"
#include "protocol_sf.h"
#include "geographic_track.h"
#include "self_check.h"
#include "mylog.hpp"

/*
cd /userdata/rk3588_83002_XTKZ_A_install/
export LD_LIBRARY_PATH=./lib/
./83002_ZK
dumpsys vpss
*/
//cd /mnt/d/XZH/项目方案/83002_xin/83002_XTKZ_A
//cd /mnt/c/Users/Administrator/Desktop/83002/83002_XTKZ_A

VPSS_ENGINE* g_vpss_engine;     //VPSS通道搭建对象
VIDEO_OSD* g_video_osd;         //视频OSD
ConfigManager* g_configManager; //系统配置文件
Vi_msg* ccdmsg;
IR_msg* irmsg;
Jgzm_msg* jgzmmsg;
SF_msg* sf_msg;
POWER_msg* power_msg;
ENV_msg* hk_msg;
XKMsg* xkMsg;
GlobalReatimeMsg* globalMsg;
BOARDABC_MSG* boardabcMsg;
TARGET_ENGINE* target_engine;
AI_ENGINE* ai_engine;
GeoGraphicTrack* geo_track;
SelfCheck* g_self_check;

int main()
{    
    RK_LOGE("29003XTKZ:2025-11-05-配置文件");
    setlocale(LC_CTYPE, "");

	// 配置文件加载
	g_configManager = new ConfigManager("config/Config.ini");
	g_configManager->init_GDConfigInfo();

    // 初始化日志模块
    // initMyLog(LogLevel::ERROR, "build");
    // LOG(INFO) << "29003_XTKZ_A System Start...";

	// 全局信息初始化
	globalMsg = new GlobalReatimeMsg();
	globalMsg->init();

	// 显控初始化
	xkMsg = new XKMsg();
	xkMsg->init_xk_socket();

	// ABC板卡网口通信
    boardabcMsg = new BOARDABC_MSG();
    boardabcMsg->Init_SOCKETA();

	// 传感器初始化 
	ccdmsg = new Vi_msg();
	irmsg = new IR_msg();
	jgzmmsg = new Jgzm_msg();
	sf_msg = new SF_msg();
	hk_msg = new ENV_msg();
	power_msg = new POWER_msg();

	// VPSS初始化
	g_vpss_engine = new VPSS_ENGINE();
	g_vpss_engine->init();
	
	// Video OSD初始化
	g_video_osd = new VIDEO_OSD();
	g_video_osd->init();
	g_video_osd->cfg->setLineColorLevel(4);
	g_video_osd->cfg->setOsdLevel(1);
	
	// 算法初始化
	target_engine = new TARGET_ENGINE();
	target_engine->init();
	ai_engine = new AI_ENGINE();
	ai_engine->init();
	geo_track = new GeoGraphicTrack();
	geo_track->init();

	// 自检模块
    g_self_check = new SelfCheck();
	g_self_check->bootcheck();                                      // 开机自检
	std::thread t_selfCheck(&SelfCheck::cyclecheck, g_self_check);  // 周期自检

    // ABC板卡通信 同步信号相关线程
    // 控制流
    // 显控消息收发线程
    std::thread t_monixk_recv(&XKMsg::recvXkThread, xkMsg);
    std::thread t_monixk_send(&XKMsg::globalRealTimeStatesUp, xkMsg);
	std::thread t_board_heartbeat(&BOARDABC_MSG::recvHeartbeatThread, boardabcMsg);
	std::thread t_c(&BOARDABC_MSG::ARecvFromBC, boardabcMsg);

    // 红外相机线程
	std::thread t_ir_recv(&IR_msg::recvIRMsgThread, irmsg);
	std::thread t_ir_query(&IR_msg::queryIRStateThread, irmsg);

    // 可见光线程
	std::thread t_vi_recv(&Vi_msg::recvCCDThread, ccdmsg);
	std::thread t_vi_query(&Vi_msg::queryStateThread, ccdmsg);

    // 激光测照器线程
	std::thread t_jgzm_recv(&Jgzm_msg::recvJGZMThread, jgzmmsg);
	std::thread t_jgzm_query(&Jgzm_msg::queryJGZMStateThread, jgzmmsg);  

    // 伺服线程 
    //std::thread t_sf_recv(&SF_msg::recvSFStateThread, sf_msg);

    // 电源模块线程
    //std::thread t_power_query(&POWER_msg::queryPowerThread, power_msg);

    // 图像流
	std::thread t_vpss_engine(&VPSS_ENGINE::run, g_vpss_engine);      // VPSS线程
	std::thread t_video_osd(&VIDEO_OSD::run, g_video_osd);            // 推流OSD线程

    // 算法任务
    std::thread t_target_engine(&TARGET_ENGINE::run, target_engine);  // 单目标跟踪
    std::thread t_ai_engine(&AI_ENGINE::run, ai_engine);              // AI目标检测

	// 环控查询和接收线程
	//std::thread t_hk_query(&ENV_msg::hkQueryThread, hk_msg);
	//std::thread t_hk_recv(&ENV_msg::hkRecvThread, hk_msg);


    while (true)
    {        
        usleep(10000); // 10ms
    }
    return 0;
}
