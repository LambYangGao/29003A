#include "rkmodel_build.h"

RK_S32 rkSysInit()
{
    avformat_network_init();

    return RK_MPI_SYS_Init();
}

RK_S32 rkSysExit()
{
    avformat_network_deinit();

    return RK_MPI_SYS_Exit();
}

RK_S32 rkOpenMediaParser(PARSER_CFG *pstParserCfg)
{
    RK_S32 s32Ret = 0;
    RK_U32 u32RetryNum = 5;
    RK_BOOL bFindStream = RK_FALSE;
    AVFormatContext *pstAvfc = RK_NULL;
    const AVStream *stream = RK_NULL;

    pstAvfc = avformat_alloc_context();

__RETRY:
    s32Ret = avformat_open_input(&(pstAvfc), pstParserCfg->srcFileUri.c_str(), NULL, NULL);
    if (s32Ret < 0)
    {
        if (s32Ret == -110 && u32RetryNum >= 0)
        {
            RK_LOGE("AGAIN");
            u32RetryNum--;
            goto __RETRY;
        }
        else
        {
            RK_LOGE("open input %s failed!", pstParserCfg->srcFileUri.c_str());
            goto __FAILED;
        }
    }

    if (avformat_find_stream_info(pstAvfc, NULL) < 0)
    {
        goto __FAILED;
    }

    RK_LOGD("found stream num: %d", pstAvfc->nb_streams);
    for (RK_S32 i = 0; i < pstAvfc->nb_streams; i++)
    {
        stream = pstAvfc->streams[i];

        if (RK_NULL != stream && stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            pstParserCfg->enCodecId = (RK_CODEC_ID_E)TEST_COMM_CodecIDFfmpegToRK(stream->codecpar->codec_id);
            pstParserCfg->u32SrcWidth = stream->codecpar->width;
            pstParserCfg->u32SrcHeight = stream->codecpar->height;
            pstParserCfg->u32StreamIndex = stream->index;
            RK_LOGD("found video stream width %d height %d", pstParserCfg->u32SrcWidth, pstParserCfg->u32SrcHeight);
            bFindStream = RK_TRUE;
            break;
        }
    }

    if (!bFindStream) {
        goto __FAILED;
    }

    RK_LOGI("open success %d", bFindStream);
    pstParserCfg->pstAvfc = pstAvfc;
    return RK_SUCCESS;

__FAILED:
    return RK_ERR_VDEC_ILLEGAL_PARAM;
}

RK_S32 rkCloseMediaParser(PARSER_CFG *pstParserCfg)
{
    if (pstParserCfg->pstAvfc)
    {
        avformat_close_input(&(pstParserCfg->pstAvfc));
    }

    return RK_SUCCESS;
}

RK_S32 rkVdecInit(VDEC_CFG *pstVdecCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;

    VDEC_CHN_ATTR_S stAttr;
    VDEC_CHN_PARAM_S stVdecParam;
    memset(&stAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    memset(&stVdecParam, 0, sizeof(VDEC_CHN_PARAM_S));

    stAttr.enMode = VIDEO_MODE_FRAME;
    stAttr.enType = pstVdecCfg->pstParserCfg->enCodecId;
    stAttr.u32PicWidth = pstVdecCfg->pstParserCfg->u32SrcWidth;
    stAttr.u32PicHeight = pstVdecCfg->pstParserCfg->u32SrcHeight;
    stAttr.u32FrameBufCnt = MAX_FRAME_QUEUE;
    stAttr.stVdecVideoAttr.bTemporalMvpEnable = RK_TRUE;

    s32Ret = RK_MPI_VDEC_CreateChn(pstVdecCfg->vdecChn, &stAttr);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("create vdec chn %d failed!", pstVdecCfg->vdecChn);
        return s32Ret;
    }

    stVdecParam.stVdecVideoParam.enCompressMode = COMPRESS_AFBC_16x16;
    s32Ret = RK_MPI_VDEC_SetChnParam(pstVdecCfg->vdecChn, &stVdecParam);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    s32Ret = RK_MPI_VDEC_StartRecvStream(pstVdecCfg->vdecChn);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    if (TEST_COMM_GetUriSchemeType(pstVdecCfg->pstParserCfg->srcFileUri.c_str()) != RK_URI_SCHEME_LOCAL)
    {
        s32Ret = RK_MPI_VDEC_SetDisplayMode(pstVdecCfg->vdecChn, VIDEO_DISPLAY_MODE_PREVIEW);
    }
    else
    {
        s32Ret = RK_MPI_VDEC_SetDisplayMode(pstVdecCfg->vdecChn, VIDEO_DISPLAY_MODE_PLAYBACK);
    }
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    RK_LOGI("create vdec chn %d success", pstVdecCfg->vdecChn);
    return RK_SUCCESS;
}

RK_S32 rkVdecDeinit(VDEC_CFG *pstVdecCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;
    s32Ret = RK_MPI_VDEC_StopRecvStream(pstVdecCfg->vdecChn);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }
    return RK_MPI_VDEC_DestroyChn(pstVdecCfg->vdecChn);
}

RK_S32 rkVpssInit(VPSS_CFG* pstVpssCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_GRP vpssGrp = pstVpssCfg->vpssGrp;
    RK_U32 u32ChnNum = pstVpssCfg->u32ChnNum;
    VPSS_CHN vpssChn = 0;

    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr[VPSS_MAX_CHN_NUM];
    memset(&stVpssGrpAttr, 0, sizeof(VPSS_GRP_ATTR_S));
    memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S) * VPSS_MAX_CHN_NUM);

    // 设置group属性
    stVpssGrpAttr.u32MaxW = pstVpssCfg->u32GrpWidth;
    stVpssGrpAttr.u32MaxH = pstVpssCfg->u32GrpHeight;
    stVpssGrpAttr.enPixelFormat = pstVpssCfg->grpPixelFormat;
    stVpssGrpAttr.enCompressMode = COMPRESS_MODE_NONE;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = pstVpssCfg->s32GrpSrcFrameRate;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = pstVpssCfg->s32GrpDstFrameRate;
    
    // 设置Chanel属性 
    for (RK_S32 i = 0; i < u32ChnNum; i++)
    {
        stVpssChnAttr[i].enChnMode = VPSS_CHN_MODE_USER;
        stVpssChnAttr[i].enCompressMode = COMPRESS_MODE_NONE;
        stVpssChnAttr[i].enPixelFormat = pstVpssCfg->chnPixelFormat[i];
        stVpssChnAttr[i].enDynamicRange = DYNAMIC_RANGE_SDR8;
        stVpssChnAttr[i].u32Width = pstVpssCfg->u32ChnWidth[i];
        stVpssChnAttr[i].u32Height = pstVpssCfg->u32ChnHeight[i];
        stVpssChnAttr[i].stFrameRate.s32SrcFrameRate = pstVpssCfg->s32ChnSrcFrameRate[i];
        stVpssChnAttr[i].stFrameRate.s32DstFrameRate = pstVpssCfg->s32ChnSrcFrameRate[i];
        stVpssChnAttr[i].u32Depth = 1;
        stVpssChnAttr[i].bFlip = RK_FALSE;
        stVpssChnAttr[i].bMirror = RK_FALSE;
    }

    // 创建Group
    s32Ret = RK_MPI_VPSS_CreateGrp(vpssGrp, &stVpssGrpAttr);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VPSS_CreateGrp (grp: %d) failed with %#x!", vpssGrp, s32Ret);
        return s32Ret;
    }

    // 设置并启用channel
    for (vpssChn = 0; vpssChn < u32ChnNum; vpssChn++)
    {
        s32Ret = RK_MPI_VPSS_SetChnAttr(vpssGrp, vpssChn, &stVpssChnAttr[vpssChn]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("RK_MPI_VPSS_SetChnAttr failed with %#x", s32Ret);
            return s32Ret;
        }

        s32Ret = RK_MPI_VPSS_EnableChn(vpssGrp, vpssChn);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("RK_MPI_VPSS_EnableChn failed with %#x", s32Ret);
            return s32Ret;
        }
    }

    // 启动Group
    s32Ret = RK_MPI_VPSS_StartGrp(vpssGrp);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VPSS_StartGrp failed with %#x", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_VPSS_ResetGrp(vpssGrp);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VPSS_ResetGrp failed with %#x", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 rkVpssDeinit(VPSS_CFG *pstVpssCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_GRP vpssGrp = pstVpssCfg->vpssGrp;
    RK_U32 u32ChnNum = pstVpssCfg->u32ChnNum;

    for (int i = 0; i < u32ChnNum; i++)
    {
        s32Ret = RK_MPI_VPSS_DisableChn(vpssGrp, i);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("RK_MPI_VPSS_DisableChn failed with %#x!", s32Ret);
            return s32Ret;
        }
    }

    s32Ret = RK_MPI_VPSS_StopGrp(vpssGrp);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VPSS_StopGrp failed with %#x!", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_VPSS_DestroyGrp(vpssGrp);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VPSS_DestroyGrp failed with %#x!", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 rkVencInit(VENC_CFG *pstVencCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VENC_CHN_ATTR_S stAttr;
    memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S));

    stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
    stAttr.stRcAttr.stH265Cbr.u32Gop = pstVencCfg->u32SrcFrameRate;
    stAttr.stRcAttr.stH265Cbr.u32SrcFrameRateNum = pstVencCfg->u32SrcFrameRate;
    stAttr.stRcAttr.stH265Cbr.u32SrcFrameRateDen = 1;
    stAttr.stRcAttr.stH265Cbr.fr32DstFrameRateNum = pstVencCfg->u32DstFrameRate;
    stAttr.stRcAttr.stH265Cbr.fr32DstFrameRateDen = 1;
    stAttr.stRcAttr.stH265Cbr.u32BitRate = pstVencCfg->u32BitRate;
    stAttr.stRcAttr.stH265Cbr.u32StatTime = 3;

    stAttr.stVencAttr.enType = (RK_CODEC_ID_E)RK_VIDEO_ID_HEVC;
    stAttr.stVencAttr.u32Profile = H265E_PROFILE_MAIN;
    stAttr.stVencAttr.enPixelFormat = pstVencCfg->chnPixelFormat;//
    stAttr.stVencAttr.u32PicWidth = pstVencCfg->u32ChnWidth;
    stAttr.stVencAttr.u32PicHeight = pstVencCfg->u32ChnHeight;
    stAttr.stVencAttr.u32VirWidth = pstVencCfg->u32ChnWidth;
    stAttr.stVencAttr.u32VirHeight = pstVencCfg->u32ChnHeight;
    stAttr.stVencAttr.enMirror = MIRROR_NONE;
    stAttr.stVencAttr.u32StreamBufCnt = 8;
    stAttr.stVencAttr.u32BufSize = pstVencCfg->u32ChnWidth * pstVencCfg->u32ChnHeight * 3;

    s32Ret = RK_MPI_VENC_CreateChn(pstVencCfg->vencChn, &stAttr);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VENC_CreateChn failed with %#x!", s32Ret);
        return s32Ret;
    }

    VENC_RECV_PIC_PARAM_S stRecvParam;
    memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));

    stRecvParam.s32RecvPicNum = -1;

    s32Ret = RK_MPI_VENC_StartRecvFrame(pstVencCfg->vencChn, &stRecvParam);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VENC_StartRecvFrame failed with %#x!", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 rkVencDeinit(VENC_CFG *pstVencCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;
    s32Ret = RK_MPI_VENC_StopRecvFrame(pstVencCfg->vencChn);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    return RK_MPI_VENC_DestroyChn(pstVencCfg->vencChn);
}

RK_S32 rkRgnInit(RGN_CFG *pstRgnCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;
    RGN_HANDLE RgnHandle = pstRgnCfg->rgnHandle;

    RGN_ATTR_S stRgnAttr;
    memset(&stRgnAttr, 0, sizeof(RGN_ATTR_S));
    stRgnAttr.enType = OVERLAY_RGN;
    stRgnAttr.unAttr.stOverlay.enPixelFmt = RK_FMT_BGRA5551;
    stRgnAttr.unAttr.stOverlay.stSize.u32Width = pstRgnCfg->u32RgnWidth;
    stRgnAttr.unAttr.stOverlay.stSize.u32Height = pstRgnCfg->u32RgnHeight;
    stRgnAttr.unAttr.stOverlay.u32CanvasNum = 2;
    stRgnAttr.unAttr.stOverlay.u32ClutNum = 0;

    s32Ret = RK_MPI_RGN_Create(RgnHandle, &stRgnAttr);
    if (RK_SUCCESS != s32Ret)
    {
        RK_LOGE("RK_MPI_RGN_Create (%d) failed with %#x!", RgnHandle, s32Ret);
        RK_MPI_RGN_Destroy(RgnHandle);
        return RK_FAILURE;
    }
    RK_LOGI("The handle: %d, create success!", RgnHandle);

    MPP_CHN_S stChn = {RK_ID_VPSS, 0, 0};
    RGN_CHN_ATTR_S stRgnChnAttr;
    memset(&stRgnChnAttr, 0, sizeof(RGN_CHN_ATTR_S));
    stRgnChnAttr.bShow = pstRgnCfg->bShow;
    stRgnChnAttr.enType = OVERLAY_RGN;
    stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = pstRgnCfg->pointX;
    stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = pstRgnCfg->pointY;
    stRgnChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 0;
    stRgnChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 255;
    stRgnChnAttr.unChnAttr.stOverlayChn.u32Layer = RgnHandle;
    stRgnChnAttr.unChnAttr.stOverlayChn.stQpInfo.bEnable = RK_FALSE;

    s32Ret = RK_MPI_RGN_AttachToChn(RgnHandle, &stChn, &stRgnChnAttr);
    if (RK_SUCCESS != s32Ret)
    {
        RK_LOGE("RK_MPI_RGN_AttachToChn (%d) failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

RK_S32 rkRgnDeinit(RGN_CFG *pstRgnCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;
    RGN_HANDLE RgnHandle = pstRgnCfg->rgnHandle;
    MPP_CHN_S stChn = {RK_ID_VPSS, 0, 0};

    s32Ret = RK_MPI_RGN_DetachFromChn(RgnHandle, &stChn);
    if (RK_SUCCESS != s32Ret)
    {
        RK_LOGE("RK_MPI_RGN_DetachFrmChn (%d) failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }
    RK_LOGI("Detach handle:%d from chn success", RgnHandle);

    s32Ret = RK_MPI_RGN_Destroy(RgnHandle);
    if (RK_SUCCESS != s32Ret)
    {
        RK_LOGE("RK_MPI_RGN_Destroy [%d] failed with %#x", RgnHandle, s32Ret);
    }
    RK_LOGI("Destory handle:%d success", RgnHandle);

    return RK_SUCCESS;
}

RK_S32 rkVdecBindVpss(RK_S32 vdecChnId, RK_S32 vpssGrpId)
{
    MPP_CHN_S stVdecChn, stVpssChn;
    stVdecChn.enModId = RK_ID_VDEC;
    stVdecChn.s32DevId = 0;
    stVdecChn.s32ChnId = vdecChnId;

    stVpssChn.enModId = RK_ID_VPSS;
    stVpssChn.s32DevId = vpssGrpId;
    stVpssChn.s32ChnId = 0;

    return RK_MPI_SYS_Bind(&stVdecChn, &stVpssChn);
}

RK_S32 rkVdecUnbindVpss(RK_S32 vdecChnId, RK_S32 vpssGrpId)
{
    MPP_CHN_S stVdecChn, stVpssChn;
    stVdecChn.enModId = RK_ID_VDEC;
    stVdecChn.s32DevId = 0;
    stVdecChn.s32ChnId = vdecChnId;

    stVpssChn.enModId = RK_ID_VPSS;
    stVpssChn.s32DevId = vpssGrpId;
    stVpssChn.s32ChnId = 0;

    return RK_MPI_SYS_UnBind(&stVdecChn, &stVpssChn);
}

RK_S32 rkVpssBindVenc(RK_S32 vpssGrpId, RK_S32 vpssChnId, RK_S32 vencChnId)
{
    MPP_CHN_S stSrcChn, stDstChn;
    stSrcChn.enModId = RK_ID_VPSS;
    stSrcChn.s32DevId = vpssGrpId;
    stSrcChn.s32ChnId = vpssChnId;

    stDstChn.enModId = RK_ID_VENC;
    stDstChn.s32DevId = vencChnId;
    stDstChn.s32ChnId = vencChnId;

    return RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
}

RK_S32 rkVpssUnbindVenc(RK_S32 vpssGrpId, RK_S32 vpssChnId, RK_S32 vencChnId)
{
    MPP_CHN_S stSrcChn, stDstChn;
    stSrcChn.enModId = RK_ID_VPSS;
    stSrcChn.s32DevId = vpssGrpId;
    stSrcChn.s32ChnId = vpssChnId;

    stDstChn.enModId = RK_ID_VENC;
    stDstChn.s32DevId = vencChnId;
    stDstChn.s32ChnId = vencChnId;

    return RK_MPI_SYS_UnBind(&stSrcChn, &stDstChn);
}

PIXEL_FORMAT_E rkGetPixelFormat(std::string format)
{
    if (format == "bgr888")
    {
        return RK_FMT_BGR888;
    }
    else if (format == "rgb888")
    {
        return RK_FMT_RGB888;
    }
    else if (format == "rgb565")
    {
        return RK_FMT_RGB565;
    }
    else if (format == "yuv422")
    {
        return RK_FMT_YUV422_YUYV;
    }
    else if (format == "yuv400")
    {
        return RK_FMT_YUV400SP;
    }
    else if (format == "yuv420")
    {
        return RK_FMT_YUV420SP;
    }
    else
    {
        RK_LOGE("not support this pixel format");
        exit(0);
    }
}

RK_S32 parseJsonCfg(GENERAL_CFG* pstGeneralCfg)
{
    Json::Reader reader;
    Json::Value root;

    std::ifstream jsonFile("config/user_config.json");
    if (!jsonFile.is_open())
    {
        RK_LOGE("open json file failed.");
        return RK_FAILURE;
    }

    if (reader.parse(jsonFile, root))
    {
        // parse vdec config 
        pstGeneralCfg->u32VdecChnNum = root["vdec"].size();

        for (int i = 0; i < pstGeneralCfg->u32VdecChnNum; i++)
        {
            std::string vdecChnName = root["vdec"].getMemberNames()[i];

            pstGeneralCfg->stParserCfgs[i].srcFileUri = root["vdec"][vdecChnName]["resource"].asString();
            pstGeneralCfg->stVdecCfgs[i].vdecChn = i;
            pstGeneralCfg->stVdecCfgs[i].pstParserCfg = &pstGeneralCfg->stParserCfgs[i];
            pstGeneralCfg->stVdecCfgs[i].u32BindGrpNum = root["vdec"][vdecChnName]["vpss_grp_bind"].size();
            for (int j = 0; j < pstGeneralCfg->stVdecCfgs[i].u32BindGrpNum; j++)
            {
                pstGeneralCfg->stVdecCfgs[i].u32BindGrp[j] = root["vdec"][vdecChnName]["vpss_grp_bind"][j].asInt();
            }
        }

        // parse vpss config
        pstGeneralCfg->u32VpssGrpNum = root["vpss"].size();
        std::string vpssChnName[4] = { "chn_0", "chn_1", "chn_2", "chn_3" };

        for (int i = 0; i < pstGeneralCfg->u32VpssGrpNum; i++)
        {
            std::string vpssGrpName = root["vpss"].getMemberNames()[i];

            pstGeneralCfg->stVpssCfgs[i].vpssGrp = i;
            pstGeneralCfg->stVpssCfgs[i].s32GrpSrcFrameRate = root["vpss"][vpssGrpName]["grp_src_frame_rate"].asInt();
            pstGeneralCfg->stVpssCfgs[i].s32GrpDstFrameRate = root["vpss"][vpssGrpName]["grp_dst_frame_rate"].asInt();
            pstGeneralCfg->stVpssCfgs[i].u32GrpWidth = root["vpss"][vpssGrpName]["grp_size"][0].asInt();
            pstGeneralCfg->stVpssCfgs[i].u32GrpHeight = root["vpss"][vpssGrpName]["grp_size"][1].asInt();
            pstGeneralCfg->stVpssCfgs[i].grpPixelFormat = rkGetPixelFormat(root["vpss"][vpssGrpName]["grp_pixel_format"].asString());

            pstGeneralCfg->stVpssCfgs[i].u32ChnNum = root["vpss"][vpssGrpName]["chn_num"].asInt();
            for (int j = 0; j < pstGeneralCfg->stVpssCfgs[i].u32ChnNum; j++)
            {
                pstGeneralCfg->stVpssCfgs[i].s32ChnSrcFrameRate[j] = root["vpss"][vpssGrpName][vpssChnName[j]]["chn_src_frame_rate"].asInt();
                pstGeneralCfg->stVpssCfgs[i].s32ChnDstFrameRate[j] = root["vpss"][vpssGrpName][vpssChnName[j]]["chn_dst_frame_rate"].asInt();
                pstGeneralCfg->stVpssCfgs[i].u32ChnWidth[j] = root["vpss"][vpssGrpName][vpssChnName[j]]["chn_scale"][0].asInt();
                pstGeneralCfg->stVpssCfgs[i].u32ChnHeight[j] = root["vpss"][vpssGrpName][vpssChnName[j]]["chn_scale"][1].asInt();
                pstGeneralCfg->stVpssCfgs[i].chnPixelFormat[j] = rkGetPixelFormat(root["vpss"][vpssGrpName][vpssChnName[j]]["chn_pixel_format"].asString());
            }
        }

        // parse venc config
        pstGeneralCfg->u32VencChnNum = root["venc"].size();

        for (int i = 0; i < pstGeneralCfg->u32VencChnNum; i++)
        {
            std::string vencChnName = root["venc"].getMemberNames()[i];

            pstGeneralCfg->stVencCfgs[i].vencChn = i;
            pstGeneralCfg->stVencCfgs[i].u32SrcFrameRate = root["venc"][vencChnName]["src_frame_rate"].asInt();
            pstGeneralCfg->stVencCfgs[i].u32DstFrameRate = root["venc"][vencChnName]["dst_frame_rate"].asInt();
            pstGeneralCfg->stVencCfgs[i].chnPixelFormat = rkGetPixelFormat(root["venc"][vencChnName]["chn_pixel_format"].asString());
            pstGeneralCfg->stVencCfgs[i].u32BitRate = root["venc"][vencChnName]["bitrate"].asInt();
            pstGeneralCfg->stVencCfgs[i].u32ChnWidth = root["venc"][vencChnName]["size"][0].asInt();
            pstGeneralCfg->stVencCfgs[i].u32ChnHeight = root["venc"][vencChnName]["size"][1].asInt();
            pstGeneralCfg->stVencCfgs[i].rtsp_ext1 = create_rtsp_demo(root["venc"][vencChnName]["rtsp_port"].asInt());
            pstGeneralCfg->stVencCfgs[i].rtsp_ext2 = create_rtsp_session(pstGeneralCfg->stVencCfgs[i].rtsp_ext1,
                root["venc"][vencChnName]["rtsp_path"].asString().c_str());
        }

        // parse rgn config
        pstGeneralCfg->u32RgnNum = root["rgn"].size();

        for (int i = 0; i < pstGeneralCfg->u32RgnNum; i++)
        {
            std::string rgnName = root["rgn"].getMemberNames()[i];

            pstGeneralCfg->stRgnCfgs[i].bShow = (RK_BOOL)(root["rgn"][rgnName]["is_show"].asBool());
            pstGeneralCfg->stRgnCfgs[i].rgnHandle = root["rgn"][rgnName]["layer"].asInt();
            pstGeneralCfg->stRgnCfgs[i].pointX = root["rgn"][rgnName]["position"][0].asInt();
            pstGeneralCfg->stRgnCfgs[i].pointY = root["rgn"][rgnName]["position"][1].asInt();
            pstGeneralCfg->stRgnCfgs[i].u32RgnWidth = root["rgn"][rgnName]["size"][0].asInt();
            pstGeneralCfg->stRgnCfgs[i].u32RgnHeight = root["rgn"][rgnName]["size"][1].asInt();
            pstGeneralCfg->stRgnCfgs[i].u32FontSize = root["rgn"][rgnName]["font_size"].asInt();
            pstGeneralCfg->stRgnCfgs[i].font_color = root["rgn"][rgnName]["font_color"].asString();
        }
    }
    else
    {
        RK_LOGE("parse json file failed.");
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

RK_S32 rkModelsInit(GENERAL_CFG *pstGeneralCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;

    // init vpss grp & chn
    for (int i = 0; i < pstGeneralCfg->u32VpssGrpNum; i++)
    {
        s32Ret = rkVpssInit(&pstGeneralCfg->stVpssCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("vpss init failed, grp: %d.", i);
            return s32Ret;
        }
    }

    // init vdec
    for (int i = 0; i < pstGeneralCfg->u32VdecChnNum; i++)
    {
        // 打开解码器
        s32Ret = rkOpenMediaParser(&pstGeneralCfg->stParserCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("open media parser failed, chn: %d.", i);
            return s32Ret;
        }

        // 初始化解码模块
        s32Ret = rkVdecInit(&pstGeneralCfg->stVdecCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("vdec init failed, chn: %d.", i);
            return s32Ret;
        }

        // 绑定 vdecChn和 vpssGrp
        for (int j = 0; j < pstGeneralCfg->stVdecCfgs[i].u32BindGrpNum; j++)
        {
            s32Ret = rkVdecBindVpss(pstGeneralCfg->stVdecCfgs[i].vdecChn, pstGeneralCfg->stVdecCfgs[i].u32BindGrp[j]);
            if (s32Ret != RK_SUCCESS)
            {
                RK_LOGE("vdec and vpss bind error.");
                return s32Ret;
            }
        }
    }

    // init venc
    for (int i = 0; i < pstGeneralCfg->u32VencChnNum; i++)
    {
        s32Ret = rkVencInit(&pstGeneralCfg->stVencCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("venc init failed, chn: %d.", i);
            return s32Ret;
        }
    }

    // init rgn
    for (int i = 0; i < pstGeneralCfg->u32RgnNum; i++)
    {
        s32Ret = rkRgnInit(&pstGeneralCfg->stRgnCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("rgn %d init failed.", i);
            return s32Ret;
        }
    }

    return s32Ret;
}

RK_S32 rkModelsDeinit(GENERAL_CFG *pstGeneralCfg)
{
    RK_S32 s32Ret = RK_SUCCESS;

    for (int i = 0; i < pstGeneralCfg->u32VdecChnNum; i++)
    {
        s32Ret = rkCloseMediaParser(&pstGeneralCfg->stParserCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("close media parser failed, chn: %d.", i);
            return s32Ret;
        }

        s32Ret = rkVdecDeinit(&pstGeneralCfg->stVdecCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("vdec deinit failed, chn: %d.", i);
            return s32Ret;
        }

        for (int j = 0; j < pstGeneralCfg->stVdecCfgs[i].u32BindGrpNum; j++)
        {
            s32Ret = rkVdecUnbindVpss(pstGeneralCfg->stVdecCfgs[i].vdecChn, pstGeneralCfg->stVdecCfgs[i].u32BindGrp[j]);
            if (s32Ret != RK_SUCCESS)
            {
                RK_LOGE("vdec and vpss unbind error.");
                return s32Ret;
            }
        }
    }

    for (int i = 0; i < pstGeneralCfg->u32VpssGrpNum; i++)
    {
        s32Ret = rkVpssDeinit(&pstGeneralCfg->stVpssCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("vpss deinit failed!");
            return s32Ret;
        }
    }

    for (int i = 0; i < pstGeneralCfg->u32VencChnNum; i++)
    {
        s32Ret = rkVencDeinit(&pstGeneralCfg->stVencCfgs[i]);
        {
            RK_LOGE("venc deinit failed!");
            return s32Ret;
        }
    }

    for (int i = 0; i < pstGeneralCfg->u32RgnNum; i++)
    {
        s32Ret = rkRgnDeinit(&pstGeneralCfg->stRgnCfgs[i]);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("rgn %d deinit failed.", i);
            return s32Ret;
        }
    }

    return s32Ret;
}
