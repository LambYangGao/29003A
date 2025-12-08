#ifndef PROTOCOL_RK3588ABC
#define PROTOCOL_RK3588ABC
#include <stdint.h>
#include "isocket.h"
#include "opencv2/opencv.hpp"
#include <thread>

#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐
/// <summary>
/// 3588A发给3588B的信息，0xFF表示无效
/// </summary>
struct BOARDA_TO_BC_MSG
{
	uint8_t HEAD1 = 0x80;
	uint8_t HEAD2 = 0x01;
	uint8_t WorkMode;         //光电主模式 0-N/A 1手动 2自动跟踪 3无人值守 4前级引导
	uint8_t MainView;         //选择主画面 1电视 2红外
	uint8_t PicControl_PIP;   //画中画 1开 2关 3画中画图像是电视 4中波红外 5短波红外 6画中画位置切换（顺时针）
	uint8_t OSD_LineColor;    //瞄准线颜色 1-白色，2-黑色，3-红色，4-绿色
	uint8_t OSD_WordDis;      //字符显示 1-字符1级显示，2-字符2级显示，3-字符3级显示
	uint8_t OSD_Level;        //OSD开启等级 0  1全开 2文字关 框开 十字开   3全关

	uint8_t AI_ONOFF;         //AI目标检测 1-开启，2-关闭
	uint8_t TARGET_ONOFF; //单目标跟踪 1-开启 2-关闭
	uint8_t Stitching_ONOFF; //视频拼接 0 不响应 1开 2关
	uint8_t Fusion_Flag;        //图像融合开关 图像融合 0-N/A 1电视与红外 2电视与SAR 3红外与SAR 4关闭
	uint8_t VisualLand_ONOFF;//视觉辅助导航开关 0 不响应 1开 2关
	uint8_t JXPP_ONOFF;        //景象匹配开关 0 不响应        1开 2关
	uint8_t Data1;//备用
	uint8_t Data2;//备用
};

//目标检测结果
struct TargetDetectRes_A2B_MSG
{
	
	uint8_t HEAD1 = 0x80;
	uint8_t HEAD2 = 0x02;
	uint16_t d_num;//目标编号
	cv::Rect bbox;//目标位置
	float f_score;//目标置信度
	uint16_t d_Target_Category;//目标类型 0车辆 1坦克
	uint16_t d_Target_Attribute;//目标属性
	bool b_center;//是否为中心目标

};


//目标跟踪结果
struct TargetTrackRes_A2B_MSG
{
	uint8_t HEAD1 = 0x80;
	uint8_t HEAD2 = 0x03;
	uint16_t trackMode = 0; //0关 1开
	int16_t trackState = 0;  //0正常 -1不正常 -2丢失
	cv::Rect predBox;          //跟踪预测框

	int16_t target_x = 0;//水平坐标差
	int16_t target_y = 0;//垂直坐标差
	float target_fw = 0.0;//水平脱靶量
	float target_fy = 0.0;//垂直脱靶量

	float  target_distance = 0;
	int32_t target_jd = 0;//目标经度
	int32_t target_wd = 0;//目标纬度
	int32_t target_gd = 0;//目标高度

	double target_ve;//目标东向速度
	double target_vn;//目标北向速度
	double target_vu;//目标天向速度  
};


// 心跳应答消息结构体
typedef struct BOARD_HEARTBEAT_RESP_MSG
{
	uint8_t HEAD1 = 0x82;  // 心跳消息头
	uint8_t HEAD2 = 0x02;  // 心跳应答类型
	uint8_t board_id;      // 应答板卡ID 2=B, 3=C
	uint8_t status;        // 0 正常 1 异常
} BOARD_HEARTBEAT_RESP_MSG;

#pragma pack(pop)

class BOARDABC_MSG
{
public:
	BOARDABC_MSG();
	~BOARDABC_MSG();

	SOCKET m_socket_A;
	SOCKET m_socket_B;

	// 心跳相关变量
	BOARD_HEARTBEAT_RESP_MSG* m_heartbeatRespMsg;
	uint64_t current_time;
	uint64_t m_last_recv_time_B;

	//A TO BC
	BOARDA_TO_BC_MSG* boardA_to_BC_Msg;
	TargetTrackRes_A2B_MSG* targetTrackRes_Msg;                        // 目标跟踪结果
	std::vector<TargetDetectRes_A2B_MSG> targetDetectRes_Msg;		   // 目标检测结果

	void initBOARDA_TO_BC_MSG();
	void initTargetTrackRes_A2B_MSG();
	void initHeartbeatMsgs();

	int Init_SOCKETA();
	int Init_SOCKETB();
	int Init_SOCKETC();
	 
	void Send_A_to_B(void* data,int len);
	void Send_B_to_A(void* data, int len);

	void ARecvFromBC();
	void BRecvFromAC();

	void SendA_to_B_Thread();//A发送B 线程
	void SendB_to_A_Thread();//B发送A 线程

	void recvHeartbeatThread();
};

#endif
