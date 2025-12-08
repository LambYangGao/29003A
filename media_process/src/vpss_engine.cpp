#include "vpss_engine.h"
#include <unistd.h>

VPSS_ENGINE::VPSS_ENGINE()
{
    cfg = new GENERAL_CFG();
}

VPSS_ENGINE::~VPSS_ENGINE()
{
    if (cfg)
    {
        delete cfg;
        cfg = nullptr;
    }
    // step5：反初始化各模块，释放相关资源
    if (rkModelsDeinit(cfg) != RK_SUCCESS)
    {
        RK_LOGE("rkmodel deinit failed!");
    }

    // step6：反初始化rkmpi系统
    if (rkSysExit() != RK_SUCCESS)
    {
        RK_LOGE("rkmpi system exit failed!");
    }
}

void VPSS_ENGINE::init()
{
    // step1：从json文件中解析配置项，配置参数写入GENERAL_CFG结构体中
    if (parseJsonCfg(cfg) != RK_SUCCESS)
    {
        RK_LOGE("parse json config failed!");
    }
    // step2：初始化rkmpi系统
    if (rkSysInit() != RK_SUCCESS)
    {
        RK_LOGE("rkmpi system init failed!");
    }
    // step3：根据配置参数，初始化各模块，搭建应用框架
    if (rkModelsInit(cfg) != RK_SUCCESS)
    {
        RK_LOGE("rkmodel init failed!");
    }
}

void VPSS_ENGINE::run()
{
    // step4：多线程处理
    // VDEC解码，发送到对应的VPSS组（测试用，不用时注释掉config里的vdec）
    if (cfg->u32VdecChnNum != 0)
    {
        RK_LOGE("VPSS_ENGINE::run VDEC %d", 0);
        stVdecSendCfg.bThreadExit = RK_FALSE;
        stVdecSendCfg.bLoop = RK_TRUE;
        stVdecSendCfg.pstParserCfg = &cfg->stParserCfgs[0];
        stVdecSendCfg.pstVdecCfg = &cfg->stVdecCfgs[0];
        vdecSendThread = std::thread(rkVdecSendStreamThread, &stVdecSendCfg); // 绑定VDEC和VPSS
    }
    else
    {
        RK_LOGE("VPSS_ENGINE::run Dma");
        for (int i = 0; i < 2; i++)
        {
            // 从PCIE/XDMA获取数据，发送到对应的VPSS组 
            stDmaRecvCfg[i].bThreadExit = RK_FALSE;
            stDmaRecvCfg[i].u32ChnFlag = i;
            stDmaRecvCfg[i].u32DmaChn=i;
            dmaRecvThread[i] = std::thread(rkDmaRecvThread, &stDmaRecvCfg[i]);
        }
    }

    //启动VENC获取线程，推流
    //可见光、红外
    for (int i = 0; i < 2; i++)
    {
		stVencGetCfg[i].bThreadExit = RK_FALSE;
		stVencGetCfg[i].pstVencCfg = &cfg->stVencCfgs[i];
		vencGetThread[i] = std::thread(rkVencGetStreamThread, &stVencGetCfg[i]);
    }

    while (1)
    {
        usleep(10);
    }
}

