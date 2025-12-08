#ifndef PROTOCOL_COMMON_H
#define PROTOCOL_COMMON_H
#include <stdint.h>
#include <mutex>
#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐

/// <summary>
/// 全局维护的状态信息
/// 主模式：0:N/A 1:手动控制 2:自动跟踪 3:无人值守 4:前级引导
/// - 手动控制：人工指令/调试；关键设备异常时保持或降级到 N/A。
/// - 自动跟踪：AI/单目标闭环；丢失≥4s或禁射区/关键设备异常→无人值守；重复异常→手动。
/// - 无人值守：自动巡航/扫描待机；有目标/雷达引导→自动跟踪；设备异常→手动。
/// - 前级引导：外部引导/从动/地理跟踪；失联或禁射区→无人值守；设备异常→手动。
/// </summary>
enum ESysWorkMode
{
    SYS_MODE_NA          = 0,
    SYS_MODE_MANUAL      = 1,
    SYS_MODE_AUTO_TRACK  = 2,
    SYS_MODE_UNATTENDED  = 3,
    SYS_MODE_PRE_GUIDE   = 4,
};

struct SystemControlMsgS
{
	uint16_t e_SysMode;           //系统工作模式 0-N/A 1-正常 2-自检 3-维护(报软件版本)
	uint16_t WorkMode;            //光电主模式 0-N/A 1-手动控制 2-自动跟踪 3-无人值守 4-前级引导 
};


//图像控制
struct PicControlMsgS
{
	uint16_t e_Switch_MainScreen; //电视-1 红外-2 
	uint16_t e_IR_NUC;                 //红外非均匀矫正 0-N/A，1-校正中，2-校正完成-----------------------------------
	uint16_t e_IR_WhiteBlack;        //红外极性切换 0-白热 1-黑热
	uint16_t e_PIP;                        //画中画 0-N/A 1开 2关 3画中画图像是电视 4中波红外 5短波红外 6画中画位置切换（顺时针）
	uint16_t e_ManualAuto;            //手动/自动 0-N/A，1-自动，2-手动
	uint16_t e_Contrast;                //图像对比度 0-N/A 1增加 2减小
	uint16_t e_Luma;                    //图像亮度 0-N/A 1增加 2减小
	uint16_t e_AutoFocal;              //一键聚焦  0-N/A，1-启动
	uint16_t e_ImageEnhance;       //图像增强  1关 2低 3中 4高 默认1
	uint16_t e_Dehaze;                 //电子透雾 0-关闭，1-打开
	uint16_t e_Switch_View;          //0-N/A，1-大视场，2-中视场，3-小视场，4-超小视场，5-电子变倍
	uint16_t e_OSD_LineColor;       //瞄准线颜色 0-N/A，1-白色，2-黑色，3-红色，4-绿色
	uint16_t e_OSD_WordDis;       //字符显示 0-N/A，1-字符1级显示，2-字符2级显示，3-字符3级显示
	uint16_t e_OSD_ShiZiSi;         //十字丝开启关闭： 关闭0 开启 1
	uint16_t e_OSD_Level;           //OSD开启等级 0  1全开 2文字关 框开 十字开   3全关
	uint16_t e_VI_Lctl;                  //低照度电视开关 0-N/A 1关 2开
};

struct AlgoControlMsg
{
	uint16_t e_AI_ONOFF;               //AI目标检测 1-开启，2-关闭
	uint16_t e_TARGET_ONOFF;           //单目标跟踪 1-开启 2-关闭
	uint16_t e_LOC_ONOFF;             //目标定位1-开启 2-关闭
	uint16_t e_Geo_ONOFF;             //地理跟踪 1-开启 2-关闭
};

struct MainViewState 
{
	int MainView = 1;
};


/// <summary>
/// 光电系统地理位置跟踪
/// </summary>
struct TrackGeoCoordinateMsg
{
	float f_TrackLongitude_value;   //地理跟踪经度 -180~180
	float f_TrackLatitude_value;    //地理跟踪纬度 -90~90
	float f_TrackAltitude_value;    //地理跟踪高度 -460~32768m 
};

/// <summary>
/// 光电电视相机信息
/// </summary>
struct CameraVIStateMsg
{
	//电视
	uint16_t s_VIWidthValue;
	uint16_t s_VIHeightValue;
	float f_VIView_Azi;               //电视方位视场角
	float f_VIView_Pitch;            //电视俯仰视场角
	uint32_t f_VIZoomValue;			 //电视焦距值
};

/// <summary>
/// 光电红外相机信息
/// </summary>
struct CameraIRStateMsg
{
	//红外
	uint16_t s_IRWidthValue;
	uint16_t s_IRHeightValue;
	float f_IRZView_Azi;       //红外方位视场角
	float f_IRZView_Pitch;     //红外俯仰视场角
	uint32_t f_IRZoomValue;    //红外焦距值
	uint16_t IRImgMode;        //红外成像模式 0黑热 1白热
};

/// <summary>
/// 光电伺服角度信息
/// </summary>
struct SFAngleMsg
{
	float f_PitchRate_value;//俯仰角速率 0°/s～20°/s
	float f_AziRate_value;//方位角速率 0°/s～60°/s
	float f_Pitch_value; //俯仰角 +30°～-135°
	float f_Azi_value;   //方位角 +180°～-180°
};

/// <summary>
/// 总体GNSS信息
/// </summary>
struct GNSSLocationPosMsg
{ 
	double d_Pitch_value;//俯仰角 -90~90抬头为正
	double d_Roll_value;//横滚角 -180~180
	double d_Heading_value;//真向角 0~360

	double d_PitchRate_value;//俯仰角速率 -180~180
	double d_RollRate_value;//横滚角速率 -180~180
	double d_Heading_Ratevalue;//航向角速率 -180~180

	double d_PitchAccele_value;//俯仰角加速度
	double d_RollAccele_value;//横滚角加速度
	double d_HeadingAccele_evalue;//航向角加速度

	double d_EastSpeed_value;//东向速度 -1024~1024
	double d_NorthSpeed_value;//北向速度 -1024~1024
	double d_UpSpeed_value;//天向速度 -1024~1024

	double d_GPS_Longitude_value; //GPS经度 -180~180
	double d_GPS_Latitude_value;   //GPS纬度 -90~90
	double d_GPS_Altitude_value;    //GPS高度（待定）

	uint16_t s_Date_Year;// 年
	uint16_t s_Date_Month;//月
	uint16_t s_Date_Day;//日
	uint16_t s_Time_Hour;// 时
	uint16_t s_Time_Min;// 分
	uint16_t s_Time_Sec;//秒
	uint16_t s_Time_ms;// 豪
	uint16_t s_Time_us;//微
};

///
///激光测照器
///
struct LaserStateMsg
{
	float f_Dis_value;           //测距距离 300~30000
	bool b_isloopDis;

	bool b_simuDis;
	float f_simu_distance;

	//State1
	int8_t    c_ld_temp;                //ld 温度数值
	uint8_t  uc_TempState;          //是否超温 0: 正常，1：异常
	uint8_t  uc_envTempState;     //环境温度(0:正常,1:异常)  
	uint8_t  uc_apdState;            //APD(0:正常, 1 : 异常)
	uint8_t  uc_mainWaveState;   // 主波(0:正常,1:异常)
	uint8_t  uc_sysStatus;            // 全系统(0:正常,1:异常)

	//State2
	uint8_t uc_workModeState;        //工作模式                              //见协议中
	uint8_t uc_prepareState;         //准备/待机(0:待机,1:准备中)
	uint8_t uc_powerState;           //上/下电(0:下电,1:上电)
	uint8_t uc_lockStatus;           // 闭锁(0:功能关,1:功能开)
};

/// <summary>
/// 舱内环控模块
/// </summary>
struct HKMsg
{
	float f_temperature;  //温度
	float f_pressure;       //气压
	float f_humidity;       //湿度
	float f_voltage;         //电压
};

/// <summary>
/// 舱外电源模块
/// </summary>
struct PowerModuleState
{
 
};

struct SensorPowerState
{
	uint8_t e_Laser_on;   //激光测照器 1物理上电 2物理下电
	uint8_t e_SF_on;       //伺服 1上电 2下电 
	uint8_t e_IR_on;       //中波红外 1上电 2下电 
	uint8_t e_CCD_on;    //电视   1上电 2下电
};

struct CycleCheckMsg
{
	uint8_t e_SocA_state;      // 主板A状态, 0-正常, 1-异常
	uint8_t e_SocB_state;      // 主板B状态, 0-正常, 1-异常
	uint8_t e_Laser_state;     // 激光测照器状态, 0-正常, 1-异常
	uint8_t e_CCD_state;       // 可见光相机状态, 0-正常, 1-异常
	uint8_t e_IR_state;        // 红外相机状态, 0-正常, 1-异常
	uint8_t e_SF_state;        // 伺服状态, 0-正常, 1-异常
	uint8_t e_Power_state;	   // 电源状态, 0-正常, 1-异常

	float temperature;         // 电源模块温度
	float proc_voltage;		   // 电源模块电流
};

#pragma pack(pop)




unsigned long getCurTickCount();

class GlobalReatimeMsg
{
public:
	GlobalReatimeMsg();
	~GlobalReatimeMsg();

	//初始化查询
	//给出部分默认值
	void init();


	void initHKMsg();
	void initGNSSLocationPosMsg();
	void initSFAngleMsg();
	void initCameraStateMsg();
	void initLaserStateMsg();
	void initPicControlMsgS();
	void initTrackGeoCoordinateMsg();
	void initMainViewState();
	void initAlgoControlMsg();
	void initSystemControlMsgS();
	void initSensorPowerState();
	void initCycleCheckMsg();


	SystemControlMsgS m_systemControlMsgS;  //系统工作模式

	HKMsg m_hkMsg;                              // 环控模块
	GNSSLocationPosMsg m_gnssMsg;               // gnss信息
	SFAngleMsg m_sfMsg;                         // 伺服模块
	CameraVIStateMsg m_camVIMsg;                // 电视相机模块
	CameraIRStateMsg m_camIRMsg;                // 红外相机模块
	LaserStateMsg m_laserStateMsg;              // 激光测照器信息
	PicControlMsgS m_picCtlMsg;                 // 图像控制
	AlgoControlMsg m_algoControlMsg;            // 算法控制
	TrackGeoCoordinateMsg m_trackGeoMsg;        // 地理引导位置
	MainViewState m_mainViewState;              // 主路视频标志位
	SensorPowerState m_senerPowerState;      	// 载荷物理上下电状态
	CycleCheckMsg m_cycleCheckMsg;           	// 周期自检信息
	
};


#endif

