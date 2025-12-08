#pragma once
#include "isocket.h"
#include "protocol_picxk.h"
#include "xuart_api.h"

void syncGlobalToBoardLan();

class XKMsg {
public:
	XKMsg();
	~XKMsg();

	SOCKET m_socket_xk;

	// 下行消息指令
	XKDownMsg* m_xkDownMsg;

	// 实时上报消息及状态消息
	PICUpRealTimeStateMsgStruct* m_picupRealTimeStateMsgStruct;
	PICUpSelfCheckStateStruct* m_picupSelfCheckStateStruct;

	int init_xk_socket();
	void initSelfCheckStateStruct();

	void processCCD(XKDownMsg * _xkDownMsg);

	void processIR(XKDownMsg* _xkDownMsg);

	void processJGCZ(XKDownMsg* _xkDownMsg);

	void processSF(XKDownMsg* _xkDownMsg);

	void processAlgo(XKDownMsg* _xkDownMsg);

	void processMainView(XKDownMsg* _xkDownMsg);

	void processPower(XKDownMsg* _xkDownMsg);

	void recvXkThread();

	// 实时消息上报
	void globalRealTimeStatesUp();

	// 简单回执（当前仅日志打印，占位）
	void sendAck(uint8_t msg_type, XKErrorCode err);
	void globalSelfCheckStatesUp();
};