#ifndef _RKMODEL_BUILD_H_
#define _RKMODEL_BUILD_H_

#include <iostream>
#include <fstream>

#include "json.h"

#include "rk_common.h"
#include "rk_debug.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"

#include "test_comm_ffm.h"
#include "test_comm_utils.h"

#include "rk_mpi_vdec.h"

#include "rk_mpi_vpss.h"
#include "rk_mpi_cal.h"
#include "test_mod_vpss.h"

#include "rk_mpi_venc.h"
#include "test_comm_venc.h"

#include "rk_mpi_rgn.h"
#include "test_comm_rgn.h"

#include "rtsp_demo.h"

#define MAX_FRAME_QUEUE 8
#define MAX_TIME_OUT_MS 200
#define VDEC_INT64_MIN (-0x7fffffffffffffffLL - 1)
#define VDEC_INT64_MAX INT64_C(9223372036854775807)

#define VDEC_CHN_NUM_LIMIT 10
#define VPSS_GRP_NUM_LIMIT 10
#define VPSS_CHN_NUM_LIMIT 4
#define VENC_CHN_NUM_LIMIT 10
#define RGN_NUM_LIMIT 10

/********************* DATA STRUCTURE ********************/
typedef struct _rkParserCfg
{
    std::string srcFileUri;
    RK_CODEC_ID_E enCodecId;
    RK_U32 u32SrcWidth;
    RK_U32 u32SrcHeight;
    RK_U32 u32StreamIndex;
    RK_U32 u32ExtraDataSize;
    RK_VOID *pExtraData;
    AVFormatContext *pstAvfc;
} PARSER_CFG;

typedef struct _rkVdecCfg
{
    VDEC_CHN vdecChn;
    RK_U32 u32BindGrpNum;
    RK_U32 u32BindGrp[VPSS_GRP_NUM_LIMIT];
    PARSER_CFG *pstParserCfg;
} VDEC_CFG;

typedef struct _rkVpssCfg
{
    VPSS_GRP vpssGrp;
    RK_S32 s32GrpSrcFrameRate;
    RK_S32 s32GrpDstFrameRate;
    PIXEL_FORMAT_E grpPixelFormat;
    RK_U32 u32GrpWidth;
    RK_U32 u32GrpHeight;

    RK_U32 u32ChnNum;
    RK_S32 s32ChnSrcFrameRate[VPSS_CHN_NUM_LIMIT];
    RK_S32 s32ChnDstFrameRate[VPSS_CHN_NUM_LIMIT];
    PIXEL_FORMAT_E chnPixelFormat[VPSS_CHN_NUM_LIMIT];
    RK_U32 u32ChnWidth[VPSS_CHN_NUM_LIMIT];
    RK_U32 u32ChnHeight[VPSS_CHN_NUM_LIMIT];
} VPSS_CFG;

typedef struct _rkVencCfg
{
    VDEC_CHN vencChn;
    RK_U32 u32SrcFrameRate;
    RK_U32 u32DstFrameRate;
    PIXEL_FORMAT_E chnPixelFormat;
    RK_U32 u32BitRate;
    RK_U32 u32ChnWidth;
    RK_U32 u32ChnHeight;
    void *rtsp_ext1;
    void *rtsp_ext2;
} VENC_CFG;

typedef struct _rkRgnCfg
{
    RK_BOOL bShow;
    RGN_HANDLE rgnHandle;
    RK_S32 pointX;
    RK_S32 pointY;
    RK_U32 u32RgnWidth;
    RK_U32 u32RgnHeight;
    RK_U32 u32FontSize;
    std::string font_color;
    std::string text;
} RGN_CFG;

typedef struct _rkGeneralCfg
{
    RK_U32 u32VdecChnNum;
    PARSER_CFG stParserCfgs[VDEC_CHN_NUM_LIMIT];
    VDEC_CFG stVdecCfgs[VDEC_CHN_NUM_LIMIT];

    RK_U32 u32VpssGrpNum;
    VPSS_CFG stVpssCfgs[VPSS_GRP_NUM_LIMIT];

    RK_U32 u32VencChnNum;
    VENC_CFG stVencCfgs[VENC_CHN_NUM_LIMIT];

    RK_U32 u32RgnNum;
    RGN_CFG stRgnCfgs[RGN_NUM_LIMIT];
} GENERAL_CFG;

/*********************** FUNCTION **********************/
RK_S32 rkSysInit();

RK_S32 rkSysExit();

RK_S32 rkOpenMediaParser(PARSER_CFG *pstParserCfg);

RK_S32 rkCloseMediaParser(PARSER_CFG *pstParserCfg);

RK_S32 rkVdecInit(VDEC_CFG *pstVdecCfg);

RK_S32 rkVdecDeinit(VDEC_CFG *pstVdecCfg);

RK_S32 rkVpssInit(VPSS_CFG *pstVpssCfg);

RK_S32 rkVpssDeinit(VPSS_CFG *pstVpssCfg);

RK_S32 rkVencInit(VENC_CFG *pstVencCfg);

RK_S32 rkVencDeinit(VENC_CFG *pstVencCfg);

RK_S32 rkRgnInit(RGN_CFG *pstRgnCfg);

RK_S32 rkRgnDeinit(RGN_CFG *pstRgnCfg);

RK_S32 rkVdecBindVpss(RK_S32 vdecChnId, RK_S32 vpssGrpId);

RK_S32 rkVdecUnbindVpss(RK_S32 vdecChnId, RK_S32 vpssGrpId);

RK_S32 rkVpssBindVenc(RK_S32 vpssGrpId, RK_S32 vpssChnId, RK_S32 vencChnId);

RK_S32 rkVpssUnbindVenc(RK_S32 vpssGrpId, RK_S32 vpssChnId, RK_S32 vencChnId);

PIXEL_FORMAT_E rkGetPixelFormat(std::string format);

RK_S32 parseJsonCfg(GENERAL_CFG *pstGeneralCfg);

RK_S32 rkModelsInit(GENERAL_CFG *pstGeneralCfg);

RK_S32 rkModelsDeinit(GENERAL_CFG *pstGeneralCfg);

#endif // _RKMODEL_BUILD_H_