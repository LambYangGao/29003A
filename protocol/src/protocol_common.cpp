#include "protocol_common.h"

GlobalReatimeMsg::GlobalReatimeMsg()
{

}

GlobalReatimeMsg::~GlobalReatimeMsg()
{

}

void GlobalReatimeMsg::init()
{
	initHKMsg();
	initGNSSLocationPosMsg();
	initSFAngleMsg();
	initCameraStateMsg();
	initLaserStateMsg();
	initPicControlMsgS();
	initTrackGeoCoordinateMsg();
	initMainViewState();
	initAlgoControlMsg();
	initSensorPowerState();
	initCycleCheckMsg();
}

void GlobalReatimeMsg::initHKMsg()
{
	m_hkMsg.f_humidity = 0;
	m_hkMsg.f_temperature = 0;
	m_hkMsg.f_pressure = 0;
	m_hkMsg.f_voltage = 0;
}

void GlobalReatimeMsg::initGNSSLocationPosMsg()
{
	m_gnssMsg.d_Pitch_value = 0;
	m_gnssMsg.d_Roll_value = 0;
	m_gnssMsg.d_Heading_value = 0;
	m_gnssMsg.d_PitchRate_value = 0;
	m_gnssMsg.d_RollRate_value = 0;
	m_gnssMsg.d_Heading_Ratevalue = 0;
	m_gnssMsg.d_PitchAccele_value = 0;
	m_gnssMsg.d_RollAccele_value = 0;
	m_gnssMsg.d_HeadingAccele_evalue = 0;
	m_gnssMsg.d_EastSpeed_value = 0;
	m_gnssMsg.d_NorthSpeed_value = 0;
	m_gnssMsg.d_UpSpeed_value = 0;
	m_gnssMsg.d_GPS_Longitude_value = 0;
	m_gnssMsg.d_GPS_Latitude_value = 0;
	m_gnssMsg.d_GPS_Altitude_value = 0;
	m_gnssMsg.s_Date_Year = 2025;
	m_gnssMsg.s_Date_Month = 1;
	m_gnssMsg.s_Date_Day = 24;
	m_gnssMsg.s_Time_Hour =14;
	m_gnssMsg.s_Time_Min = 29;
	m_gnssMsg.s_Time_Sec = 42;
	m_gnssMsg.s_Time_ms = 666;
	m_gnssMsg.s_Time_us = 666;
}

void GlobalReatimeMsg::initSFAngleMsg()
{
	m_sfMsg.f_PitchRate_value = 3;
	m_sfMsg.f_AziRate_value = 0;
	m_sfMsg.f_Pitch_value = 0;
	m_sfMsg.f_Azi_value = 0;
}
void GlobalReatimeMsg::initCameraStateMsg()
{
	/*相机参数*/
	//电视
	m_camVIMsg.s_VIHeightValue = 1080;
	m_camVIMsg.s_VIWidthValue = 1920;
	m_camVIMsg.f_VIView_Azi = 37.41;
	m_camVIMsg.f_VIView_Pitch = 19;
	m_camVIMsg.f_VIZoomValue = 202;

	//红外
	m_camIRMsg.s_IRWidthValue = 640;
	m_camIRMsg.s_IRHeightValue = 512;
	m_camIRMsg.f_IRZView_Azi = 35.0;
	m_camIRMsg.f_IRZView_Pitch = 22;
	m_camIRMsg.f_IRZoomValue = 100;
	m_camIRMsg.IRImgMode = 1;   //白热
}
void GlobalReatimeMsg::initLaserStateMsg()
{
	m_laserStateMsg.f_Dis_value = 0;
	m_laserStateMsg.b_isloopDis = false;
	m_laserStateMsg.f_simu_distance = 0;
	m_laserStateMsg.b_simuDis = false;
	m_laserStateMsg.c_ld_temp = 0;
	m_laserStateMsg.uc_TempState = 0;
	m_laserStateMsg.uc_envTempState = 0;
	m_laserStateMsg.uc_apdState = 0;
	m_laserStateMsg.uc_mainWaveState = 0;
	m_laserStateMsg.uc_sysStatus = 0;
	m_laserStateMsg.uc_workModeState = 0;
	m_laserStateMsg.uc_prepareState = 0;
	m_laserStateMsg.uc_powerState = 0;
	m_laserStateMsg.uc_lockStatus = 0;
}
void GlobalReatimeMsg::initPicControlMsgS()
{
	//图像控制相关
	m_picCtlMsg.e_Switch_MainScreen = 1;//电视/红外 0 1电视 2红外 
	m_picCtlMsg.e_IR_NUC = 1;//红外非均匀矫正 0-N/A，1-否，2-是
	m_picCtlMsg.e_IR_WhiteBlack = 1;//红外极性切换 0 1白热 2黑热
	m_picCtlMsg.e_PIP = 1;//画中画 0 1开 2关 3画中画图像是电视 4中波红外 5短波红外 6画中画位置切换（顺时针）
	m_picCtlMsg.e_ManualAuto = 1;    //手动/自动  0-N/A，1-自动，2-手动
	m_picCtlMsg.e_Contrast = 0;        //图像对比度 0 1增加 2减小
	m_picCtlMsg.e_Luma = 0;            //图像亮度 0 1增加 2减小
	m_picCtlMsg.e_AutoFocal = 0;//一键聚焦  0-N/A，1-启动
	m_picCtlMsg.e_ImageEnhance = 1;//图像增强 0 1关 2低 3中 4高 默认1
	m_picCtlMsg.e_Dehaze = 0;           //电子透雾 0 1关 2低 3中 4高 默认1
	m_picCtlMsg.e_Switch_View = 1;    //0-N/A，1-大视场，2-中视场，3-小视场，4-超小视场，5-电子变倍
	m_picCtlMsg.e_OSD_LineColor = 4;//瞄准线颜色 0-N/A，1-白色，2-黑色，3-红色，4-绿色
	m_picCtlMsg.e_OSD_WordDis = 1;//字符显示 0-N/A，1-字符1级显示，2-字符2级显示，3-字符3级显示
	m_picCtlMsg.e_OSD_Level = 1;    //OSD开启等级 0  1全开 2文字关 框开 十字开   3全关
	m_picCtlMsg.e_VI_Lctl = 1;        //低照度电视开关 0 1关 2开
}
void GlobalReatimeMsg::initTrackGeoCoordinateMsg()
{
	m_trackGeoMsg.f_TrackAltitude_value = 0;
	m_trackGeoMsg.f_TrackLatitude_value = 0;
	m_trackGeoMsg.f_TrackLongitude_value = 0;
}
void GlobalReatimeMsg::initMainViewState()
{
	m_mainViewState.MainView = 1;
}

void GlobalReatimeMsg::initAlgoControlMsg()
{
	//算法初始化状态均为关闭
	m_algoControlMsg.e_AI_ONOFF = 2;
	m_algoControlMsg.e_Geo_ONOFF = 2;
	m_algoControlMsg.e_TARGET_ONOFF = 2;
}

void GlobalReatimeMsg::initSystemControlMsgS()
{
	m_systemControlMsgS.e_SysMode = 1;
	m_systemControlMsgS.WorkMode = SYS_MODE_MANUAL;
}

void GlobalReatimeMsg::initSensorPowerState()
{
	m_senerPowerState.e_CCD_on = 2;
	m_senerPowerState.e_IR_on = 2;
	m_senerPowerState.e_Laser_on = 2;
	m_senerPowerState.e_SF_on = 2;
}

void GlobalReatimeMsg::initCycleCheckMsg()
{
	m_cycleCheckMsg.e_SocA_state = 0;      // 默认正常
	m_cycleCheckMsg.e_SocB_state = 0;
	m_cycleCheckMsg.e_CCD_state = 1;
	m_cycleCheckMsg.e_IR_state = 1;
	m_cycleCheckMsg.e_SF_state = 1;
	m_cycleCheckMsg.e_Power_state = 0;
}

unsigned long getCurTickCount()
{
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);//此处可以判断一下返回
    return (ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000));
}
