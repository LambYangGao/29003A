#include "video_osd.h"
#include "osd_process.h"


#include <unistd.h>
#include <algorithm>
#include <thread>

//#include <target_engine.h>
//#include <protocol_tjqh.h>
//#include "ai_engine.h"
//#include "smoke_detect.h"
//#include <protocol_rk3588abc.h>
extern GlobalReatimeMsg* globalMsg;
extern BOARDABC_MSG* boardabcMsg;

VIDEO_OSD::VIDEO_OSD()
{
    cfg = new VideoOsdCfg();
}

VIDEO_OSD::~VIDEO_OSD()
{
    if (cfg)
    {
        delete cfg;
        cfg = nullptr;
    }
}

void VIDEO_OSD::init()
{
    cfg->setMainView(1);
    cfg->setOsdLevel(3);
    cfg->setLineColorLevel(4);
}

void VIDEO_OSD::run()
{
    // 创建两个处理线程
    std::thread thread_tv([this]() { processVideoStream(0); });   // 可见光
    std::thread thread_ir([this]() { processVideoStream(1); });   // 红外
    
    thread_tv.join();
    thread_ir.join();

}


void VIDEO_OSD::processVideoStream(int streamIdx)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VIDEO_FRAME_INFO_S inferFrame;
    cv::Mat image;
    std::vector<DetBox> detBoxs;
    std::vector<OsdTextInfoCtx> textCtxs;
    
    const char* streamName = (streamIdx == 0) ? "VI" : "IR";
    RK_LOGE("VIDEO_OSD stream %s started", streamName);

    while (!cfg->bThreadExit)
    {
        memset(&inferFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
        detBoxs.clear();
        textCtxs.clear();
        
        // 从对应的VPSS通道获取帧
        s32Ret = RK_MPI_VPSS_GetChnFrame(
            cfg->u32VpssGroup[streamIdx], 
            cfg->u32VpssChn[streamIdx], 
            &inferFrame, 
            cfg->s32TimeOutMs
        );
       
        if (s32Ret != RK_SUCCESS) {
            //RK_LOGE("VIDEO_OSD RK_MPI_VPSS_GetChnFrame failed!");
            //RK_MPI_VPSS_ReleaseChnFrame(cfg->u32VpssGroup, cfg->u32VpssChn, &inferFrame);
            continue;
        }

        // 只对主视图进行OSD处理
        bool isMainView = (cfg->u32MainView == (streamIdx + 1));
        
        if (isMainView && boardabcMsg->boardA_to_BC_Msg->AI_ONOFF == 1)
        {
            // OSD目标检测框处理
            if (cfg->bOsdRect)
            {
                for (int i = 0; i < boardabcMsg->targetDetectRes_Msg.size(); i++)
                {
                    DetBox tempBox;
                    tempBox.x = boardabcMsg->targetDetectRes_Msg[i].bbox.x;
                    tempBox.y = boardabcMsg->targetDetectRes_Msg[i].bbox.y;
                    tempBox.width = boardabcMsg->targetDetectRes_Msg[i].bbox.width;
                    tempBox.height = boardabcMsg->targetDetectRes_Msg[i].bbox.height;
                    
                    // 尺寸校验
                    if (tempBox.y > cfg->u32Height[streamIdx]) {
                        continue;
                    }
                    
                    detBoxs.push_back(tempBox);
                    
                    // 添加文字标注
                    OsdTextInfoCtx tempText;
                    tempText.x = tempBox.x;
                    tempText.y = tempBox.y;
                    tempText.fontSize = 20;
                    tempText.text = std::to_string(i + 1);
                    
                    switch (boardabcMsg->targetDetectRes_Msg[i].d_Target_Category)
                    {
                        case 0: tempText.text += "船"; break;
                        case 1: tempText.text += "浮标"; break;
                        case 2: tempText.text += "人员"; break;
                        case 3: tempText.text += "车辆"; break;
                        case 4: tempText.text += "建筑"; break;
                        default: tempText.text += "其它"; break;
                    }
                    textCtxs.push_back(tempText);
                }
                
                if (!detBoxs.empty())
                {
                    rkCvRealRect(&inferFrame, detBoxs, cfg->u32LineColor, 2);
                    rkCvOsdText(&inferFrame, textCtxs, cfg->u32LineColor);
                }
            }
            
            // 目标跟踪框处理
            if (boardabcMsg->boardA_to_BC_Msg->TARGET_ONOFF == 1)
            {
                std::vector<DetBox> trackBoxs;
                DetBox trackBox;
                trackBox.x = boardabcMsg->targetTrackRes_Msg->predBox.x;
                trackBox.y = boardabcMsg->targetTrackRes_Msg->predBox.y;
                trackBox.width = boardabcMsg->targetTrackRes_Msg->predBox.width;
                trackBox.height = boardabcMsg->targetTrackRes_Msg->predBox.height;
                trackBoxs.push_back(trackBox);
                rkCvRealRect(&inferFrame, trackBoxs, cfg->u32LineColor, 2);
            }
        }
        
        // 十字准星处理（主视图）
        if (isMainView && cfg->bOsdCross)
        {
            DetBox crossBox;
            if (streamIdx == 0) {  // 可见光
                crossBox = {910, 490, 100, 100};
            } else {  // 红外
                crossBox = {320, 256, 50, 50};
            }
            std::vector<DetBox> crossBoxs;
            crossBoxs.push_back(crossBox);
            rkCvFalseCross(&inferFrame, crossBoxs, cfg->u32LineColor, 2);
        }

        // 发送到对应的VENC通道
        s32Ret = RK_MPI_VENC_SendFrame(cfg->u32VencChn[streamIdx], &inferFrame, -1);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Stream %s RK_MPI_VENC_SendFrame failed(%#x)", streamName, s32Ret);
        }

        // 释放帧
        s32Ret = RK_MPI_VPSS_ReleaseChnFrame(
            cfg->u32VpssGroup[streamIdx], 
            cfg->u32VpssChn[streamIdx], 
            &inferFrame
        );
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Stream %s ReleaseChnFrame failed!", streamName);
        }
    }
    
    RK_LOGD("VIDEO_OSD stream %s out", streamName);
}