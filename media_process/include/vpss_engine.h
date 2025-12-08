#ifndef VPSS_ENGINE_H
#define VPSS_ENGINE_H

#include <stdint.h>
#include <string>
#include "rkmodel_build.h"
#include <thread>
#include <vector>
#include "thread_process.h"


class VPSS_ENGINE
{
public:
	VPSS_ENGINE();
	~VPSS_ENGINE();

	//配置参数 
	GENERAL_CFG* cfg = nullptr;//vpss系统配置参数

	VDEC_SEND_CFG stVdecSendCfg;//vdec解码配置参数
	std::thread vdecSendThread;

	VENC_GET_CFG stVencGetCfg[2];//venc处理配置参数
	std::thread vencGetThread[2];
	
	//RGN_UPDATE_CFG stRgnUpdateCfg;//rgn文字配置参数
	//std::thread rgnUpdateThread;

	DMA_RECV_CFG stDmaRecvCfg[2];//DMA图像数据接收配置参数
	std::thread dmaRecvThread[2];
	//运行函数
	void init();
	void run();
};

#endif //VPSS_ENGINE_H