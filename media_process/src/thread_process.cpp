#include "thread_process.h"
#include "video_io.h"

RK_S32 mpi_ffmpeg_free(void *opaque)
{
    AVPacket *avPkt = reinterpret_cast<AVPacket *>(opaque);
    if (RK_NULL != avPkt)
    {
        av_packet_unref(avPkt);
        av_packet_free(&avPkt);
    }
    avPkt = RK_NULL;
    return RK_SUCCESS;
}

RK_VOID rkVdecSendStreamThread(VDEC_SEND_CFG *pstVdecSendCfg)
{
    RK_LOGD("%s in", __FUNCTION__);

    RK_S32 s32Ret = 0;
    RK_U64 u64PTS = 0;
    VDEC_STREAM_S stStream;
    AVStream *vStream = RK_NULL;
    MB_BLK buffer = RK_NULL;
    MB_EXT_CONFIG_S stMbExtConfig;
    AVPacket *avPacket = RK_NULL;
    RK_BOOL bEos = RK_FALSE;
    RK_BOOL bFindKeyFrame = RK_FALSE;
    RK_BOOL bFirstFrame = RK_TRUE;

    memset(&stStream, 0, sizeof(VDEC_STREAM_S));
    memset(&stMbExtConfig, 0, sizeof(MB_EXT_CONFIG_S));

    while (!pstVdecSendCfg->bThreadExit)
    {
        avPacket = av_packet_alloc();
        av_init_packet(avPacket);
        if (bFirstFrame && pstVdecSendCfg->pstParserCfg->u32ExtraDataSize > 0)
        {
            avPacket->data = reinterpret_cast<uint8_t *>(pstVdecSendCfg->pstParserCfg->pExtraData);
            avPacket->size = pstVdecSendCfg->pstParserCfg->u32ExtraDataSize;
            avPacket->stream_index = pstVdecSendCfg->pstParserCfg->u32StreamIndex;
            avPacket->flags = AV_PKT_FLAG_KEY;
            bFirstFrame = RK_FALSE;
        }
        else
        {
            s32Ret = av_read_frame(pstVdecSendCfg->pstParserCfg->pstAvfc, avPacket);
            if (s32Ret == AVERROR_EOF || s32Ret == AVERROR_EXIT)
            {
                mpi_ffmpeg_free(avPacket);
                if (pstVdecSendCfg->bLoop == false)
                {
                    break;
                }

                RK_LOGD("eos %d ret", pstVdecSendCfg->pstVdecCfg->vdecChn);
                avformat_seek_file(pstVdecSendCfg->pstParserCfg->pstAvfc,
                                   pstVdecSendCfg->pstParserCfg->u32StreamIndex,
                                   VDEC_INT64_MIN, 0, VDEC_INT64_MAX, AVSEEK_FLAG_BYTE);

                bEos = RK_TRUE;
                bFindKeyFrame = RK_FALSE;
                continue;
            }
        }

        if (pstVdecSendCfg->pstParserCfg->u32StreamIndex == avPacket->stream_index)
        {
            if (!bFindKeyFrame)
            {
                if (avPacket->flags & AV_PKT_FLAG_KEY == AV_PKT_FLAG_KEY)
                {
                    bFindKeyFrame = RK_TRUE;
                }
                else
                {
                    mpi_ffmpeg_free(avPacket);
                    continue;
                }
            }

            if (bEos)
            {
                RK_LOGD("entern send %d", pstVdecSendCfg->pstVdecCfg->vdecChn);
                bEos = RK_FALSE;
            }

            stMbExtConfig.pFreeCB = mpi_ffmpeg_free;
            stMbExtConfig.pOpaque = avPacket;
            stMbExtConfig.pu8VirAddr = avPacket->data;
            stMbExtConfig.u64Size = avPacket->size;

            RK_MPI_SYS_CreateMB(&buffer, &stMbExtConfig);

            vStream = pstVdecSendCfg->pstParserCfg->pstAvfc->streams[avPacket->stream_index];
            u64PTS = av_q2d(vStream->time_base) * (avPacket->pts) * 1000000ll;
            stStream.u64PTS = u64PTS;
            stStream.pMbBlk = buffer;
            stStream.u32Len = avPacket->size;
            stStream.bEndOfStream = RK_FALSE;
            stStream.bEndOfFrame = RK_FALSE;
            stStream.bBypassMbBlk = RK_TRUE;

        __RETRY:
            s32Ret = RK_MPI_VDEC_SendStream(pstVdecSendCfg->pstVdecCfg->vdecChn, &stStream, MAX_TIME_OUT_MS);
            if (s32Ret != RK_SUCCESS)
            {
                if (pstVdecSendCfg->bThreadExit)
                {
                    RK_LOGE("failed to RK_MPI_VDEC_SendStream");
                    mpi_ffmpeg_free(avPacket);
                    RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
                    break;
                }
                usleep(10000llu);
                goto __RETRY;
            }

            RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
        }
        else
        {
            mpi_ffmpeg_free(avPacket);
        }
    }

    rkCloseMediaParser(pstVdecSendCfg->pstParserCfg);

__FAILED:
    return;
}

//RK_VOID rkRgnUpdateThread(RGN_UPDATE_CFG *pstRgnUpdateCfg)
//{
//    RK_LOGD("%s in", __FUNCTION__);
//
//    RK_S32 s32Ret = RK_SUCCESS;
//    clock_t start, end;
//
//    while (!pstRgnUpdateCfg->bThreadExit)
//    {
//        start = clock();
//
//        // 位图更新
//        for (int i = 0; i < 4; i++)
//        {
//            RGN_HANDLE RgnHandle;
//            RgnHandle = pstRgnUpdateCfg->pstRgnCfg[i].rgnHandle;
//
//            RGN_CANVAS_INFO_S stCanvasInfo;
//            memset(&stCanvasInfo, 0, sizeof(RGN_CANVAS_INFO_S));
//
//            s32Ret = RK_MPI_RGN_GetCanvasInfo(RgnHandle, &stCanvasInfo);
//            if (s32Ret != RK_SUCCESS)
//            {
//                RK_LOGE("RK_MPI_RGN_GetCanvasInfo failed with %#x!", s32Ret);
//                return;
//            }
//
//            s32Ret = rkText2BmpData(&pstRgnUpdateCfg->pstRgnCfg[i], reinterpret_cast<void *>(stCanvasInfo.u64VirAddr));
//
//            s32Ret = RK_MPI_RGN_UpdateCanvas(RgnHandle);
//            if (s32Ret != RK_SUCCESS)
//            {
//                RK_LOGE("RK_MPI_RGN_UpdateCanvas failed with %#x!", s32Ret);
//                return;
//            }
//        }
//
//        end = clock();
//        while (end - start < 1000000)
//        {
//            end = clock();
//        }
//    }
//
//    RK_LOGD("%s out\n", __FUNCTION__);
//    return;
//}

RK_VOID rkVencGetStreamThread(VENC_GET_CFG *pstVencGetCfg)
{
    RK_LOGD("%s in", __FUNCTION__);

    char *pData = RK_NULL;
    RK_S32 s32Ret = 0;
    RK_U32 u32Chn = pstVencGetCfg->pstVencCfg->vencChn;
    RK_S32 s32StreamCnt = 0;

    VENC_STREAM_S stFrame;
    memset(&stFrame, 0, sizeof(VENC_STREAM_S));
    stFrame.pstPack = reinterpret_cast<VENC_PACK_S *>(malloc(sizeof(VENC_PACK_S)));

    rtsp_demo_handle g_rtsplive = pstVencGetCfg->pstVencCfg->rtsp_ext1;
    rtsp_session_handle session = pstVencGetCfg->pstVencCfg->rtsp_ext2;

    while (!pstVencGetCfg->bThreadExit)
    {
        stFrame.u32PackCount = 1;
        s32Ret = RK_MPI_VENC_GetStream(u32Chn, &stFrame, -1);
        if (s32Ret >= 0)
        {
            s32StreamCnt++;

            // add rtsp
            pData = (char *)RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);

            rtsp_sever_tx_video(g_rtsplive, session,
                                (uint8_t *)pData,
                                stFrame.pstPack->u32Len,
                                rtsp_get_reltime());

            rtsp_do_event(g_rtsplive);

            RK_MPI_VENC_ReleaseStream(u32Chn, &stFrame);
            if (stFrame.pstPack->bStreamEnd == RK_TRUE)
            {
                RK_LOGI("chn %d reach EOS stream", u32Chn);
                break;
            }
        }
        else
        {
            if (pstVencGetCfg->bThreadExit)
            {
                break;
            }
            RK_LOGE("chn(%d) get stream err(0x%x)", u32Chn, s32Ret);
            usleep(1000llu);
        }
    }

    if (stFrame.pstPack)
    {
        free(stFrame.pstPack);
    }

    RK_LOGD("%s out\n", __FUNCTION__);
    return;
}

bool testsave = false;
RK_VOID rkDmaRecvThread(DMA_RECV_CFG* pstDmaRecvCfg)
{
    RK_LOGD("%s in", __FUNCTION__);

    RK_S32 s32Ret = 0;
    RK_U32 u32Width, u32Height;
    void* data = NULL;
    uint64_t size = 0;

    int fpga_fd = -1;
    //int current_index;
    //char file_flag = 0;
    //int out_file_fd = 0, ret;

    // 初始化XDMA
    VideoXMDA* video_xdma = new VideoXMDA();
    fpga_fd=video_xdma->init_xmda(pstDmaRecvCfg->u32DmaChn);
    
    RK_LOGE("test");
    while (!pstDmaRecvCfg->bThreadExit)
    {
        // 此处根据u32DmaChn选择接收哪一路的数据，确定好图像尺寸以及数据标志位和地址
        switch (pstDmaRecvCfg->u32ChnFlag)
        {
        case 0: // 电视
        {
            u32Width = 1920;
            u32Height = 1080;
            size = u32Width * u32Height * 2;
            break;
        }
        case 1: // 中波红外
        {
            u32Width = 640;
            u32Height = 512;
            size = u32Width * u32Height *3/2;
            break;
        }
        default:
        {
            break;
        }
        }
        
        // 构造视频帧结构
        VIDEO_FRAME_INFO_S stVideoFrame = { 0 };
        stVideoFrame.stVFrame.u32Width = u32Width;
        stVideoFrame.stVFrame.u32Height = u32Height;
        stVideoFrame.stVFrame.u32VirWidth = u32Width;
        stVideoFrame.stVFrame.u32VirHeight = u32Height;
        stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
        stVideoFrame.stVFrame.enPixelFormat = pstDmaRecvCfg->u32ChnFlag ==0? RK_FMT_YUV422_YUYV: RK_FMT_YUV420P;  //红外为单通道640x512
        stVideoFrame.stVFrame.pMbBlk = RK_NULL;

        PIC_BUF_ATTR_S stBufAttr = { 0 };
        stBufAttr.enCompMode = COMPRESS_MODE_NONE;
        stBufAttr.enPixelFormat = pstDmaRecvCfg->u32ChnFlag == 0 ? RK_FMT_YUV422_YUYV : RK_FMT_YUV420P;
        stBufAttr.u32Width = u32Width;
        stBufAttr.u32Height = u32Height;

        MB_PIC_CAL_S stCalResult;
        RK_MPI_CAL_VGS_GetPicBufferSize(&stBufAttr, &stCalResult);

        // 分配帧内存
        s32Ret = RK_MPI_SYS_MmzAlloc_Cached(&(stVideoFrame.stVFrame.pMbBlk), RK_NULL, RK_NULL, stCalResult.u32MBSize);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("RK_MPI_SYS_MmzAllocc_Cached failed(%#x)", s32Ret); 
            continue;
        }

        // 从XDMA读取数据
        data = RK_MPI_MB_Handle2VirAddr(stVideoFrame.stVFrame.pMbBlk);
        int res = video_xdma->readImgBuf(pstDmaRecvCfg->u32ChnFlag, fpga_fd, data, size);

        // xvideo_read(fpga_fd, (char*)data, 640*512);
        // memset(data + 640 * 512, 0x80, 640 * 512 / 2);

        RK_MPI_SYS_MmzFlushCache(stVideoFrame.stVFrame.pMbBlk, RK_FALSE);

        //将接收的数据发送到VPSS
        if (res > 0)
		{
			RK_MPI_VPSS_SendFrame(pstDmaRecvCfg->u32ChnFlag, 0, &stVideoFrame, -1);    //发到group0 算法调用产生检测结果
        }

        // 释放内存
        RK_MPI_SYS_MmzFree(stVideoFrame.stVFrame.pMbBlk);
    }

    video_xdma->close_fd(fpga_fd);
    delete video_xdma;
    video_xdma = nullptr;
}

