#ifndef _THREAD_PROCESS_
#define _THREAD_PROCESS_

#include <unistd.h>
#include <time.h>

#include "rtsp_demo.h"
#include "rkmodel_build.h"
#include "osd_process.h"
//#include "recv_from_fpga.h"
#include "xvideo_api.h"

#define IMAGE_FLAG_0 0
#define IMAGE_FLAG_1 1
#define IMAGE_DATA_0 0
#define IMAGE_DATA_1 1

typedef struct _vdecSendCfg
{
    RK_BOOL bThreadExit;
    RK_BOOL bLoop;
    PARSER_CFG *pstParserCfg;
    VDEC_CFG *pstVdecCfg;
} VDEC_SEND_CFG;

typedef struct _vencGetCfg
{
    RK_BOOL bThreadExit;
    VENC_CFG *pstVencCfg;
} VENC_GET_CFG;

typedef struct _rgnUpdateCfg
{
    RK_BOOL bThreadExit;
    RK_U32 u32UpdateTime;
    RGN_CFG *pstRgnCfg;
} RGN_UPDATE_CFG;
    
typedef struct _dmaRecvCfg
{
    RK_BOOL bThreadExit;
    RK_U32 u32ChnFlag;// u32ChnFlag == 0主路 1画中画
    RK_U32 u32DmaChn;// u32DmaChn ==  0 电视 1红外
    RK_U32 u32MainView;// u32MainView == 1 电视 2红外
    void setMainView(int num)
    {
        u32MainView = num;
        switch (u32MainView)
        {
        case 1:
            u32DmaChn = 0;
            break;
        case 2:
            u32DmaChn = 1;
            break;
        default:
            break;
        }
    }
} DMA_RECV_CFG;

RK_S32 mpi_ffmpeg_free(void *opaque);

RK_VOID rkVdecSendStreamThread(VDEC_SEND_CFG *pstVdecSendCfg);

//RK_VOID rkRgnUpdateThread(RGN_UPDATE_CFG *pstRgnUpdateCfg);

RK_VOID rkVencGetStreamThread(VENC_GET_CFG *pstVencGetCfg);

RK_VOID rkDmaRecvThread(DMA_RECV_CFG* pstDmaRecvCfg);

#endif // _THREAD_PROCESS_
