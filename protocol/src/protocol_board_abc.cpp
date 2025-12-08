#include <string.h>
#include <string>
#include <stdio.h>
#include <rk_debug.h>

#include "protocol_board_abc.h"
#include "protocol_common.h"
#include "mylog.hpp"
#include "ConfigManager.h"
extern ConfigManager* g_configManager;
extern GlobalReatimeMsg* globalMsg;

BOARDABC_MSG::BOARDABC_MSG()
{
	boardA_to_BC_Msg = new BOARDA_TO_BC_MSG();
	targetTrackRes_Msg = new TargetTrackRes_A2B_MSG();
	m_heartbeatRespMsg = new BOARD_HEARTBEAT_RESP_MSG();
	
	initBOARDA_TO_BC_MSG();
	initTargetTrackRes_A2B_MSG();
	initHeartbeatMsgs();

	current_time = 0;
	m_last_recv_time_B = 0;

}

BOARDABC_MSG::~BOARDABC_MSG()
{
	if (boardA_to_BC_Msg)
	{
		delete boardA_to_BC_Msg;
		boardA_to_BC_Msg = NULL;
	}
	if (targetTrackRes_Msg)
	{
		delete targetTrackRes_Msg;
		targetTrackRes_Msg = NULL;
	}
	if (m_heartbeatRespMsg) {
		delete m_heartbeatRespMsg;
		m_heartbeatRespMsg = NULL;
	}
}

void BOARDABC_MSG::initBOARDA_TO_BC_MSG()
{
	boardA_to_BC_Msg->HEAD1 = 0x80;
	boardA_to_BC_Msg->HEAD2 = 0x01;
	boardA_to_BC_Msg->WorkMode = 0;
	boardA_to_BC_Msg->MainView = 1;

	boardA_to_BC_Msg->PicControl_PIP = 2;
	boardA_to_BC_Msg->OSD_LineColor = 4;
	boardA_to_BC_Msg->OSD_WordDis = 1;
	boardA_to_BC_Msg->OSD_Level = 1;

	boardA_to_BC_Msg->AI_ONOFF = 2;
	boardA_to_BC_Msg->TARGET_ONOFF = 2;
	boardA_to_BC_Msg->Stitching_ONOFF = 2;
	boardA_to_BC_Msg->Fusion_Flag = 4;
	boardA_to_BC_Msg->VisualLand_ONOFF = 2;
	boardA_to_BC_Msg->JXPP_ONOFF = 2;
	boardA_to_BC_Msg->Data1 = 2;
	boardA_to_BC_Msg->Data2 = 2;
}

void BOARDABC_MSG::initTargetTrackRes_A2B_MSG()
{
	targetTrackRes_Msg->HEAD1 = 0x80;
	targetTrackRes_Msg->HEAD2 = 0x03;
	targetTrackRes_Msg->trackMode =0;
	targetTrackRes_Msg->trackState = 0;
	targetTrackRes_Msg->predBox = cv::Rect(0,0,0,0);
	targetTrackRes_Msg->target_fw = 0;
	targetTrackRes_Msg->target_fy = 0;
	targetTrackRes_Msg->target_jd = 0;
	targetTrackRes_Msg->target_wd = 0;
	targetTrackRes_Msg->target_gd = 0;
	targetTrackRes_Msg->target_ve = 0;
	targetTrackRes_Msg->target_vn = 0;
	targetTrackRes_Msg->target_vu = 0;
}

void BOARDABC_MSG::initHeartbeatMsgs()
{
	m_heartbeatRespMsg->HEAD1 = 0x82;
	m_heartbeatRespMsg->HEAD2 = 0x02;
	m_heartbeatRespMsg->board_id = 1; // A°å
	m_heartbeatRespMsg->status = 0;
}


int BOARDABC_MSG::Init_SOCKETA()
{
	m_socket_A = iSocket::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int ret = iSocket::bind(m_socket_A, g_configManager->m_gd_configinfo->B_Send_A_Port);
	if (-1 == ret) {
		perror("init failed! ");
	}
	return 0;
}

int BOARDABC_MSG::Init_SOCKETB()
{
	m_socket_B= iSocket::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int ret = iSocket::bind(m_socket_B, g_configManager->m_gd_configinfo->A_Send_B_Port);
	if (-1 == ret) {
		perror("init failed! ");
	}
	return 0;
}

void BOARDABC_MSG::Send_A_to_B(void* data, int len)
{
	iSocket::sendto_some(m_socket_A, data, len, g_configManager->m_gd_configinfo->A_Send_B_Port,
		g_configManager->m_gd_configinfo->GD_B_IP);
}

void BOARDABC_MSG::Send_B_to_A(void* data, int len)
{
	iSocket::sendto_some(m_socket_B, data, len, g_configManager->m_gd_configinfo->B_Send_A_Port,
		g_configManager->m_gd_configinfo->GD_A_IP);
}

void BOARDABC_MSG::ARecvFromBC()
{
    uint8_t buffer[4096] = { 0 };
    while (1)
    {
        memset(buffer, 0, 4096);
        int ret = iSocket::recv_some(m_socket_A, buffer, 4096);
        if (ret > 0)
        {

        }
        else
        {
            usleep(1000);
        }
    }
}

void BOARDABC_MSG::BRecvFromAC()
{

}

void BOARDABC_MSG::SendA_to_B_Thread()
{

}

void BOARDABC_MSG::SendB_to_A_Thread()
{

}


void BOARDABC_MSG::recvHeartbeatThread()
{
	uint8_t buffer[4096] = { 0 };
	const int64_t MAX_TIME_DIFF = 5000;  // 5Ãë
	sockaddr* addr = NULL;
	while (1)
	{
		memset(buffer, 0, 4096);
		//int ret = iSocket::recv_some(m_socket_A, buffer, 4096);
		int ret = iSocket::recv_some_noblock(m_socket_A, buffer, 4096, addr);
		if (ret > 0) {
			if (ret == sizeof(BOARD_HEARTBEAT_RESP_MSG)) {
				memcpy(m_heartbeatRespMsg, buffer, sizeof(BOARD_HEARTBEAT_RESP_MSG));

				current_time = getCurTickCount();
				if (m_heartbeatRespMsg->HEAD1 == 0x82 && m_heartbeatRespMsg->HEAD2 == 0x02) {
					if (m_heartbeatRespMsg->board_id == 2) { // B°å
						if (m_last_recv_time_B != 0)
						{
							int64_t time_diff = current_time - m_last_recv_time_B;
							if (time_diff > MAX_TIME_DIFF)
							{
								globalMsg->m_cycleCheckMsg.e_SocB_state = 1;
							}
							else
							{
								globalMsg->m_cycleCheckMsg.e_SocB_state = 0;

							}
						}
						m_last_recv_time_B = current_time;
						//RK_LOGE("Recv B heartbeatRespMsg");

					}
				}
			}
		}
		else
		{
			usleep(10000);
		}
	}
}
