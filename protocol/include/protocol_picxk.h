#ifndef PROTOCOL_PICXK
#define PROTOCOL_PICXK
#include <stdint.h>
#include "protocol_common.h"

#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐

/// <summary>
/// 控制消息结构体-显控下发给图像板
/// </summary>
typedef struct XKDownMsg
{
	uint8_t sync_code1; //0xEB
	uint8_t sync_code2; //0x90
	uint8_t msg_type;//消息类型
	int32_t param_1; //消息参数
	int32_t param_2;
	int32_t param_3;
	int32_t param_4;
	int32_t param_5;
}XKDownMsg;

typedef struct XKDownConfig
{
	uint8_t sync_code1 = 0xEB;  //0xEB
	uint8_t sync_code2 = 0x90;  //0x90
	uint8_t msg_type = 0x05;	//E_FK_SYS_SET_ONE_CONFIG
	int32_t key_len;
	char key[250];
	int32_t value_len;
	char value[250];
}XKDownConfig;

/// <summary>
/// 下行/上行统一错误码
/// </summary>
typedef enum XKErrorCode
{
	XK_ERR_OK          = 0,  // 成功
	XK_ERR_INVALID_ARG = 1,  // 参数非法/越界
	XK_ERR_UNSUPPORTED = 2,  // 不支持的命令或当前模式不允许
	XK_ERR_HW_FAULT    = 3,  // 关键硬件故障
	XK_ERR_BUSY        = 4,  // 设备忙/正在执行
} XKErrorCode;

/// <summary>
/// 通用应答帧（上行回执）
/// </summary>
typedef struct XKAckMsg
{
	uint8_t sync_code1;  // 0xEB
	uint8_t sync_code2;  // 0x90
	uint8_t msg_type;    // 回显下行 msg_type
	uint8_t result;      // 0 成功 1 失败
	uint8_t err_code;    // XKErrorCode
	uint8_t reserved[3]; // 对齐/扩展
} XKAckMsg;

/// <summary>
/// 控制消息类型
/// </summary>
typedef enum XKMsgType
{
	/* 0x00-0x06 系统 / 管理 */
	E_FK_SYS_SELFCHECK = 0x00,     // 自检
	E_FK_SYS_LOG = 0x01,           // 日志存储  param1:开关/等级
	E_FK_SYS_VERSION = 0x02,       // 读取软件版本
	E_FK_SWITCH_VL = 0x03,         // 切换到可见光
	E_FK_SWITCH_IR = 0x04,         // 切换到红外
	E_FK_SYS_CONFIG_INFO = 0x05,   // 查询下位机配置参数
	E_FK_SET_TIME = 0x06,          // 设置系统时间 yyyyMMddhhmmss

	/* 0x07-0x1F 可见光相机控制 */
	E_FK_VL_OPEN = 0x07,               // 可见光上电
	E_FK_VL_CLOSE = 0x08,              // 可见光下电
	E_FK_VL_DEHAZE_OPEN = 0x09,        // 可见光去雾开
	E_FK_VL_DEHAZE_CLOSE = 0x0A,       // 可见光去雾关
	E_FK_VL_IR_OPEN = 0x0B,            // 可见光红外模式(黑夜)
	E_FK_VL_IR_CLOSE = 0x0C,           // 可见光非红外模式(白天)
	E_FK_VI_FOCUSE_AUTO = 0x0D,        // 聚焦自动
	E_FK_VI_FOCUSE_MANUL = 0x0E,       // 聚焦手动
	E_FK_VI_FOCUSE_SEMIAUTO = 0x0F,    // 聚焦半自动
	E_FK_VL_LUMA_SET_LEVEL = 0x10,     // 手动设置亮度等级
	E_FK_VL_CONTRAST_INCREASES = 0x11, // 手动调节模式下 对比度增加
	E_FK_VL_CONTRAST_DECREASES = 0x12, // 手动调节模式下 对比度减弱
	E_FK_VL_OSD_CLOSE = 0x13,          // 十字丝-关
	E_FK_VL_OSD_OPEN = 0x14,           // 十字丝-开
	E_FK_VL_STOP_ZOOM = 0x15,          // 停止变倍/聚焦
	E_FK_AUTO_FOCUSING = 0x16,         // 一键自动调焦
	E_FK_VL_ZOOM_INCREASES = 0x17,     // 变倍+
	E_FK_VL_ZOOM_DECREASES = 0x18,     // 变倍-
	E_FK_VL_FOCUSING_INCREASES = 0x19, // 聚焦+
	E_FK_VL_FOCUSING_DECREASES = 0x1A, // 聚焦-
	E_FK_VL_RotationHorizontalOn = 0x1B, // 水平翻转开
	E_FK_Vl_RotationHorizontalOff = 0x1C, // 水平翻转关
	E_FK_VL_RotationVerticallyOn = 0x1D, // 垂直翻转开
	E_FK_Vl_RotationVerticallyOff = 0x1E, // 垂直翻转关
	E_FK_VI_SEND_CUSTOMDATA = 0x1F,      // 发送自定义指令

	/* 0x20-0x30 红外相机控制 */
	E_FK_IR_ZOOM_INCREASES = 0x20,       // 变倍+
	E_FK_IR_ZOOM_DECREASES = 0x21,       // 变倍-
	E_FK_IR_FOCUSING_INCREASES = 0x22,   // 聚焦+
	E_FK_IR_FOCUSING_DECREASES = 0x23,   // 聚焦-
	E_FK_IR_OPEN = 0x24,                 // 红外上电
	E_FK_IR_CLOSE = 0x25,                // 红外下电  
	E_FK_IR_BG_CORRECTION = 0x26,        // 红外背景校正
	E_FK_IR_MANUL_CORRECTION = 0x27,     // 红外快门校正
	E_FK_IR_IMAGE_ENHANCEMENT_CLOSE = 0x28, // 红外图像增强关闭
	E_FK_IR_IMAGE_ENHANCEMENT_1 = 0x29,  // 图像增强 1档
	E_FK_IR_IMAGE_ENHANCEMENT_2 = 0x2A,  // 图像增强 2档
	E_FK_IR_IMAGE_ENHANCEMENT_3 = 0x2B,  // 图像增强 3档
	E_FK_IR_STOP_ZOOM = 0x2C,            // 停止变倍/聚焦
	E_FK_IR_OSD_CLOSE = 0x2D,            // 十字丝-关
	E_FK_IR_OSD_OPEN = 0x2E,             // 十字丝-开
	E_FK_IR_SEND_CUSTOMDATA = 0x2F,      // 红外自定义指令
	E_FK_IR_AUTO_FOCUSING = 0x30,        // 红外自动调焦

	/* 0x31-0x3F 伺服 */
	E_FK_SF_OPEN = 0x31,           // 伺服上电 
	E_FK_SF_CLOSE = 0x32,          // 伺服下电 
	E_FK_SF_STOP = 0x33,           // 伺服停车
	E_FK_SF_SHOUC = 0x34,          // 收藏 镜头向上收起
	E_FK_SF_MOVE = 0x35,           // 手动加矩 速度环控制
	E_FK_SF_MOVE_TO = 0x36,        // 数引 指定方位俯仰
	E_FK_SF_SAOMIAO_INFO = 0x37,   // 扇扫参数设置
	E_FK_SF_RESTORE = 0x38,        // 归中 伺服归零  
	E_FK_SF_FWAUTO = 0x39,         // 方位扫描
	E_FK_SF_FYMANUL = 0x3A,        // 俯仰手动
	E_FK_GYRO_UPPLEFT = 0x3B,      // 左上移动
	E_FK_GYRO_UPPRIGHT = 0x3C,     // 右上移动
	E_FK_GYRO_LOWERLEFT = 0x3D,    // 左下移动
	E_FK_GYRO_LOWERRIGHT = 0x3E,   // 右下移动
	E_FK_SF_ZHOUSAO = 0x3F,        // 周扫

	/* 0x40-0x44 激光测距 */
	E_FK_LASER_RANGING_CLOSE = 0x40,  // 激光测距下电
	E_FK_LASER_RANGING_OPEN = 0x41,   // 激光测距上电
	E_FK_LASER_RANGING_ONE = 0x42,    // 单次测距
	E_FK_LASER_RANGING_MANY = 0x43,   // 连续测距
	E_FK_LASER_RANGING_STOP = 0x44,   // 停止测距

	/* 0x45-0x57 算法 / 视频处理 */
	E_FK_TK_SINGLE_OPEN = 0x45,     // 单目标跟踪开启
	E_FK_TK_SINGLE_CLOSE = 0x46,    // 单目标跟踪关闭 
	E_FK_TK_POSITION_OPEN = 0x47,   // 目标定位开启
	E_FK_TK_POSITION_CLOSE = 0x48,  // 目标定位关闭
	E_FK_TK_SPEED_OPEN = 0x49,      // 目标速度测量开启
	E_FK_TK_SPEED_CLOSE = 0x4A,     // 目标速度测量关闭
	E_FK_GEO_FOLLOW_OPEN = 0x4B,    // 从动地理坐标开启  
	E_FK_GEO_FOLLOW_CLOSE = 0x4C,   // 从动地理坐标关闭
	E_FK_AI_DETECT_OPEN = 0x4D,     // AI目标检测开启
	E_FK_AI_DETECT_CLOSE = 0x4E,    // AI目标检测关闭
	E_FK_FUSION_IR_VL = 0x4F,       // 红外与可见光图像融合
	E_FK_FUSION_IR_SAR = 0x50,      // 红外与SAR雷达融合
	E_FK_FUSION_VL_SAR = 0x51,      // 可见光与SAR雷达融合
	E_FK_VIDEO_STITCH_ON = 0x52,    // 视频图像拼接开启
	E_FK_VIDEO_ENCODE = 0x53,       // 视频编码
	E_FK_VIDEO_STORAGE = 0x54,      // 视频存储(压缩)
	E_FK_OSD_OVERLAY = 0x55,        // OSD叠加
	E_FK_SCENE_MATCH_ON = 0x56,     // 景象匹配与定位开启
	E_FK_VISUAL_LANDING_ON = 0x57,  // 视觉辅助导航降落开启

	/* 0x58-0x70 显示 / 存储 / 电源 */
	E_FK_OPEN_PIP = 0x58,           // 画中画开
	E_FK_CLOSE_PIP = 0x59,          // 画中画关
	E_FK_GRAB_PIC = 0x5A,           // 抓图/单拍
	E_FK_GRAB_PICS_START = 0x5B,    // 开始连拍 周期4s
	E_FK_GRAB_PICS_STOP = 0x5C,     // 停止拍照
	E_FK_SWITCH_IR_VL = 0x5D,       // 切换到红外可见双路输出
	E_FK_STOP_RECORD = 0x5E,        // 停止录像
	E_FK_START_RECORD = 0x5F,       // 开始录像
	E_FK_IR_SWITCH = 0x60,          // 切换到红外视屏
	E_FK_VI_SWITCH = 0x61,          // 切换到可见光并开始录像
	E_FK_VI_Brightness = 0x62,    // 可见光亮度值
	E_FK_VI_Contrast = 0x63,     // 可见光对比度值
	E_FK_VI_Saturation = 0x64,     // 可见光饱和度值
	E_FK_VI_Sharpness = 0x65,     // 可见光锐度值

	E_FK_LASER_Lock_ON = 0x66,    // 激光测照闭锁开
    E_FK_LASER_Lock_OFF = 0x67,   // 激光测照闭锁关
    E_FK_LASER_setEnergy = 0x68,   // 激光测照能量设置

	E_FK_FUSION_OFF=0x69,               //融合关闭
	E_FK_VISUAL_LANDING_OFF=0x6A,      //视觉辅助导航降落关闭
	E_FK_SCENE_MATCH_OFF =0x6B,        //景象匹配关闭
	E_FK_VIDEO_STITCH_OFF=0x6C,         //视频拼接关闭

	E_FK_LASER_ON =  0x6D,           //激光测距机物理上电
	E_FK_LASER_OFF = 0x6E,           //激光测距机物理下电
	E_FK_SAR_ON = 0x6F,              //SAR物理上电
	E_FK_SAR_OFF = 0x70,             //SAR物理下电
	E_FK_BUTT = 0xFF                 // 无意义
}XKMsgType;

/// <summary>
/// 接收消息结构体
/// </summary>
typedef enum PICUpMessageTypeEnum
{
	FKUP_REALTIME_STATE_E = 0x0,//实时状态
	FKUP_SELFCHECK_STATE_E = 0x1,//自检信息
	FKUP_CONFIG_INFO_E = 0x2,//配置参数
	FKUP_DWDX_INFO_E = 0x3,//北斗信息
	FKUP_MAX_STATE_E = 0xff,
}PICUpMsgTypeE;

/// <summary>
/// 接收PIC-实时状态
/// </summary>
typedef struct PICUpRealTimeStateMsgStruct //25hz
{
	uint8_t sync_code;//0x90
	uint8_t type;//FKUP_REALTIME_STATE_E

	//载荷上下电状态
    uint8_t e_Laser_on;    //激光测照器 1物理上电 2物理下电
	uint8_t e_SF_on;       //伺服 1上电 2下电 
    uint8_t e_IR_on;       //中波红外 1上电 2下电 
    uint8_t e_CCD_on;      //电视   1上电 2下电

	//伺服
    float f_PitchRate_value;   //俯仰角速率 0°/s～20°/s
    float f_AziRate_value;    //方位角速率 0°/s～60°/s
    float f_Pitch_value;      //俯仰角 +30°～-135°
    float f_Azi_value;         //方位角 +180°～-180°
	uint8_t state;//伺服控制组件 D0 方位编码器； D1 俯仰编码器； D2 方位驱动器； D3 俯仰驱动器； D4 伺服控制板； D5 方位陀螺仪； D6 俯仰陀螺仪
	uint8_t mode;//伺服状态1进行0无效(默认) D0 俯仰扇形扫描； D1 方位扇形扫描 ；D2随动； D3 跟踪； D4 收藏；D5 归零位； D6 伺服停止； D7 方位圆周扫描 
	
    //电视   
    float f_VIView_Azi;               //电视方位视场角
    float f_VIView_Pitch;            //电视俯仰视场角
    uint32_t f_VIZoomValue;          //电视焦距值

	//红外
    float f_IRZView_Azi;            //红外方位视场角
    float f_IRZView_Pitch;          //红外俯仰视场角
    uint32_t f_IRZoomValue;          //红外焦距值

	//测照器
	float laser_distance;
	bool  laser_isloopDis;
    uint8_t laser_workModeState;        //工作模式                             
    uint8_t laser_prepareState;         //准备/待机(0:待机,1:准备中)
    uint8_t laser_powerState;           //上/下电(0:下电,1:上电)
    uint8_t laser_lockStatus;           // 闭锁(0:功能关,1:功能开)

	//算法开启关闭状态
    uint16_t e_AI_ONOFF;               //AI目标检测 1-开启，2-关闭
    uint16_t e_TARGET_ONOFF;           //单目标跟踪 1-开启 2-关闭
    uint16_t e_LOC_ONOFF;             //目标定位1-开启 2-关闭
    uint16_t e_Geo_ONOFF;             //地理跟踪 1-开启 2-关闭
}PICUpStateMsgS;

/// <summary>
/// 接收PIC-自检状态信息
/// </summary>
typedef struct PICUpSelfCheckStateStruct//30s1次
{
	uint8_t sync_code;//0xEB90
	uint8_t type;//FKUP_REALTIME_STATE_E
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

#endif
