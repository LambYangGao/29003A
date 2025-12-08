#include "osd_process.h"

cv::Scalar convertColor(RK_U32 color)
{
    // 提取颜色分量
    unsigned char r = (color >> 16) & 0xFF;
    unsigned char g = (color >> 8) & 0xFF;
    unsigned char b = color & 0xFF;

    // 创建 cv::Scalar, OpenCV 使用 BGR 顺序
    return cv::Scalar(b, g, r);
}

//RK_VOID rkSetTextColor(SDL_Color *pTextColor, std::string colorName)
//{
//    if (colorName == "black")
//    {
//        *pTextColor = {128, 128, 128, 128};
//    }
//    else if (colorName == "white")
//    {
//        *pTextColor = {255, 255, 255, 128};
//    }
//    else if (colorName == "red")
//    {
//        *pTextColor = {128, 128, 255, 128};
//    }
//    else if (colorName == "green")
//    {
//        *pTextColor = {128, 255, 128, 128};
//    }
//    else if (colorName == "blue")
//    {
//        *pTextColor = {255, 128, 128, 128};
//    }
//    else if (colorName == "yellow")
//    {
//        *pTextColor = {128, 255, 255, 128};
//    }
//    else
//    {
//        RK_LOGE("not support this font color, default color: white.");
//        *pTextColor = {255, 255, 255, 128};
//    }
//
//    return;
//}

RK_S32 rkVgsOsdFrame(VIDEO_FRAME_INFO_S *pOuterFrame, VIDEO_FRAME_INFO_S *pInnerFrame, PointXY point)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VGS_HANDLE handle;
    VGS_ADD_OSD_S osdInfo;
    VGS_TASK_ATTR_S task = {0};

    memcpy(&task.stImgIn, pOuterFrame, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&task.stImgOut, pOuterFrame, sizeof(VIDEO_FRAME_INFO_S));

    s32Ret = RK_MPI_VGS_BeginJob(&handle);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    osdInfo.stRect.s32X = point.x;
    osdInfo.stRect.s32Y = point.y;
    osdInfo.stRect.u32Width = pInnerFrame->stVFrame.u32Width;
    osdInfo.stRect.u32Height = pInnerFrame->stVFrame.u32Height;
    osdInfo.enPixelFmt = pInnerFrame->stVFrame.enPixelFormat;
    osdInfo.pMbBlk = pInnerFrame->stVFrame.pMbBlk;

    s32Ret = RK_MPI_VGS_AddOsdTask(handle, &task, &osdInfo);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VGS_AddOsdTaskArray failed! ret:%#x", s32Ret);
        RK_MPI_VGS_CancelJob(handle);
    }

    s32Ret = RK_MPI_VGS_EndJob(handle);
    if (s32Ret != RK_SUCCESS)
    {
        RK_MPI_VGS_CancelJob(handle);
    }

    RK_MPI_SYS_MmzFlushCache(pOuterFrame->stVFrame.pMbBlk, RK_FALSE);

    return 0;
}

RK_S32 rkVgsRealRect(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VGS_HANDLE handle;

    VGS_TASK_ATTR_S task = {0};
    VGS_DRAW_LINE_S line[100] = {0};

    memcpy(&task.stImgIn, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&task.stImgOut, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S));

    s32Ret = RK_MPI_VGS_BeginJob(&handle);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    int count = 0;
    for (auto &box : detBoxs)
    {
        // 绘制框的上横线
        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = box.y / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        // 绘制框的下横线
        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        // 绘制框的左竖线
        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = box.x / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        // 绘制框的右竖线
        line[count].stStartPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;
    }

    s32Ret = RK_MPI_VGS_AddDrawLineTaskArray(handle, &task, line, count);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VGS_AddDrawLineTaskArray failed! ret:%#x", s32Ret);
        RK_MPI_VGS_CancelJob(handle);
    }

    s32Ret = RK_MPI_VGS_EndJob(handle);

    if (s32Ret != RK_SUCCESS)
    {
        RK_MPI_VGS_CancelJob(handle);
    }

    return 0;
}

RK_S32 rkVgsFalseRect(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VGS_HANDLE handle;

    VGS_TASK_ATTR_S task = {0};
    VGS_DRAW_LINE_S line[100] = {0};

    memcpy(&task.stImgIn, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&task.stImgOut, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S));

    s32Ret = RK_MPI_VGS_BeginJob(&handle);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    int count = 0;
    for (auto &box : detBoxs)
    {
        int door_len = (box.height > box.width) ? (box.width / 4) : (box.height / 4);
        // 绘制框的上横线
        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + door_len) / 2 * 2;
        line[count].stEndPoint.s32Y = box.y / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        line[count].stStartPoint.s32X = (box.x + box.width - door_len) / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = box.y / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        // 绘制框的下横线
        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + door_len) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        line[count].stStartPoint.s32X = (box.x + box.width - door_len) / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        // 绘制框的左竖线
        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = box.x / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + door_len) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height - door_len) / 2 * 2;
        line[count].stEndPoint.s32X = box.x / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        // 绘制框的右竖线
        line[count].stStartPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + door_len) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        line[count].stStartPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height - door_len) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;
    }

    s32Ret = RK_MPI_VGS_AddDrawLineTaskArray(handle, &task, line, count);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VGS_AddDrawLineTaskArray failed! ret:%#x", s32Ret);
        RK_MPI_VGS_CancelJob(handle);
    }

    s32Ret = RK_MPI_VGS_EndJob(handle);

    if (s32Ret != RK_SUCCESS)
    {
        RK_MPI_VGS_CancelJob(handle);
    }

    return 0;
}

RK_S32 rkVgsRealCross(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VGS_HANDLE handle;

    VGS_TASK_ATTR_S task = {0};
    VGS_DRAW_LINE_S line[100] = {0};

    memcpy(&task.stImgIn, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&task.stImgOut, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S));

    s32Ret = RK_MPI_VGS_BeginJob(&handle);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    int count = 0;
    for (auto &box : detBoxs)
    {
        // 绘制框的横线
        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height / 2) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height / 2) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        // 绘制框的竖线
        line[count].stStartPoint.s32X = (box.x + box.width / 2) / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width / 2) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;
    }

    s32Ret = RK_MPI_VGS_AddDrawLineTaskArray(handle, &task, line, count);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VGS_AddDrawLineTaskArray failed! ret:%#x", s32Ret);
        RK_MPI_VGS_CancelJob(handle);
    }

    s32Ret = RK_MPI_VGS_EndJob(handle);

    if (s32Ret != RK_SUCCESS)
    {
        RK_MPI_VGS_CancelJob(handle);
    }

    return 0;
}

RK_S32 rkVgsFalseCross(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VGS_HANDLE handle;

    VGS_TASK_ATTR_S task = {0};
    VGS_DRAW_LINE_S line[100] = {0};

    memcpy(&task.stImgIn, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&task.stImgOut, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S));

    s32Ret = RK_MPI_VGS_BeginJob(&handle);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    int count = 0;
    for (auto &box : detBoxs)
    {
        /// 绘制框的左横线
        line[count].stStartPoint.s32X = box.x / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height / 2) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width / 4) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height / 2) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        /// 绘制框的右横线
        line[count].stStartPoint.s32X = (box.x + box.width * 3 / 4) / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height / 2) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height / 2) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        /// 绘制框的上竖线
        line[count].stStartPoint.s32X = (box.x + box.width / 2) / 2 * 2;
        line[count].stStartPoint.s32Y = box.y / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width / 2) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height / 4) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        /// 绘制框的下竖线
        line[count].stStartPoint.s32X = (box.x + box.width / 2) / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height * 3 / 4) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width / 2) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        /// 绘制框的中心点
        line[count].stStartPoint.s32X = (box.x + box.width / 2 - 2) / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height / 2) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width / 2 + 2) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height / 2) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;

        line[count].stStartPoint.s32X = (box.x + box.width / 2) / 2 * 2;
        line[count].stStartPoint.s32Y = (box.y + box.height / 2 - 2) / 2 * 2;
        line[count].stEndPoint.s32X = (box.x + box.width / 2) / 2 * 2;
        line[count].stEndPoint.s32Y = (box.y + box.height / 2 + 2) / 2 * 2;
        line[count].u32Color = u32Color;
        line[count].u32Thick = u32Thick;
        count++;
    }

    s32Ret = RK_MPI_VGS_AddDrawLineTaskArray(handle, &task, line, count);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VGS_AddDrawLineTaskArray failed! ret:%#x", s32Ret);
        RK_MPI_VGS_CancelJob(handle);
    }

    s32Ret = RK_MPI_VGS_EndJob(handle);

    if (s32Ret != RK_SUCCESS)
    {
        RK_MPI_VGS_CancelJob(handle);
    }

    return RK_SUCCESS;
}

RK_S32 rkCvRealRect(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick)
{
    cv::Mat bgrImg;
    int width = pstFrameInfo->stVFrame.u32Width;
    int height = pstFrameInfo->stVFrame.u32Height;

    bgrImg = cv::Mat(height, width, CV_8UC3, pstFrameInfo->stVFrame.pVirAddr[0]);
    if (bgrImg.empty())
    {
        RK_LOGE("Failed to create cvImage.");
        return RK_FAILURE;
    }

    cv::Scalar color = convertColor(u32Color);

    for (auto &detBox : detBoxs)
    {
        int x = detBox.x;
        int y = detBox.y;
        int width = detBox.width;
        int height = detBox.height;

        cv::rectangle(bgrImg, cv::Point(x, y), cv::Point(x + width, y + height), color, u32Thick);
    }

    RK_MPI_SYS_MmzFlushCache(pstFrameInfo->stVFrame.pMbBlk, RK_FALSE);

    return RK_SUCCESS;
}

RK_S32 rkCvFalseRect(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick)
{
    cv::Mat bgrImg;
    int width = pstFrameInfo->stVFrame.u32Width;
    int height = pstFrameInfo->stVFrame.u32Height;

    bgrImg = cv::Mat(height, width, CV_8UC3, pstFrameInfo->stVFrame.pVirAddr[0]);
    if (bgrImg.empty())
    {
        RK_LOGE("Failed to create cvImage.");
        return RK_FAILURE;
    }

    cv::Scalar color = convertColor(u32Color);

    for (auto &detBox : detBoxs)
    {
        int x = detBox.x;
        int y = detBox.y;
        int width = detBox.width;
        int height = detBox.height;
        int door_len = (detBox.height > detBox.width) ? (detBox.width / 4) : (detBox.height / 4);

        // 左上角
        cv::line(bgrImg, cv::Point(x, y), cv::Point(x + door_len, y), color, u32Thick);
        cv::line(bgrImg, cv::Point(x, y), cv::Point(x, y + door_len), color, u32Thick);

        // 左下角
        cv::line(bgrImg, cv::Point(x, y + height), cv::Point(x + door_len, y + height), color, u32Thick);
        cv::line(bgrImg, cv::Point(x, y + height), cv::Point(x, y + height - door_len), color, u32Thick);

        // 右上角
        cv::line(bgrImg, cv::Point(x + width, y), cv::Point(x + width - door_len, y), color, u32Thick);
        cv::line(bgrImg, cv::Point(x + width, y), cv::Point(x + width, y + door_len), color, u32Thick);

        // 右下角
        cv::line(bgrImg, cv::Point(x + width, y + height), cv::Point(x + width - door_len, y + height), color, u32Thick);
        cv::line(bgrImg, cv::Point(x + width, y + height), cv::Point(x + width, y + height - door_len), color, u32Thick);
    }

    RK_MPI_SYS_MmzFlushCache(pstFrameInfo->stVFrame.pMbBlk, RK_FALSE);

    return RK_SUCCESS;
}

RK_S32 rkCvRealCross(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick)
{
    cv::Mat bgrImg;
    int width = pstFrameInfo->stVFrame.u32Width;
    int height = pstFrameInfo->stVFrame.u32Height;

    bgrImg = cv::Mat(height, width, CV_8UC3, pstFrameInfo->stVFrame.pVirAddr[0]);
    if (bgrImg.empty())
    {
        RK_LOGE("Failed to create cvImage.");
        return RK_FAILURE;
    }

    cv::Scalar color = convertColor(u32Color);

    for (auto &detBox : detBoxs)
    {
        int x = detBox.x;
        int y = detBox.y;
        int width = detBox.width;
        int height = detBox.height;

        cv::line(bgrImg, cv::Point(x - width / 2, y), cv::Point(x + width / 2, y), color, u32Thick);
        cv::line(bgrImg, cv::Point(x, y - height / 2), cv::Point(x, y + height / 2), color, u32Thick);
    }

    RK_MPI_SYS_MmzFlushCache(pstFrameInfo->stVFrame.pMbBlk, RK_FALSE);

    return RK_SUCCESS;
}

RK_S32 rkCvFalseCross(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick)
{
    cv::Mat bgrImg;
    int width = pstFrameInfo->stVFrame.u32Width;
    int height = pstFrameInfo->stVFrame.u32Height;

    bgrImg = cv::Mat(height, width, CV_8UC3, pstFrameInfo->stVFrame.pVirAddr[0]);
    if (bgrImg.empty())
    {
        RK_LOGE("Failed to create cvImage.");
        return RK_FAILURE;
    }

    cv::Scalar color = convertColor(u32Color);

    for (auto &detBox : detBoxs)
    {
        int x = detBox.x;
        int y = detBox.y;
        int width = detBox.width;
        int height = detBox.height;

        int door_len = (detBox.height > detBox.width) ? (detBox.width / 3) : (detBox.height / 3);

        cv::circle(bgrImg, cv::Point(x, y), u32Thick * 2, color, cv::FILLED);

        // 上线
        cv::line(bgrImg, cv::Point(x, y - height / 2), cv::Point(x, y - height / 2 + door_len), color, u32Thick);

        // 下线
        cv::line(bgrImg, cv::Point(x, y + height / 2 - door_len), cv::Point(x, y + height / 2), color, u32Thick);

        // 左线
        cv::line(bgrImg, cv::Point(x - width / 2, y), cv::Point(x - width / 2 + door_len, y), color, u32Thick);

        // 右线
        cv::line(bgrImg, cv::Point(x + width / 2 - door_len, y), cv::Point(x + width / 2, y), color, u32Thick);
    }

    RK_MPI_SYS_MmzFlushCache(pstFrameInfo->stVFrame.pMbBlk, RK_FALSE);

    return RK_SUCCESS;
}

RK_S32 rkCvOsdText(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<OsdTextInfoCtx> textCtxs, RK_U32 u32Color)
{
    cv::Mat bgrImg;
    int width = pstFrameInfo->stVFrame.u32Width;
    int height = pstFrameInfo->stVFrame.u32Height;

    bgrImg = cv::Mat(height, width, CV_8UC3, pstFrameInfo->stVFrame.pVirAddr[0]);
    if (bgrImg.empty())
    {
        RK_LOGE("Failed to create cvImage.");
        return RK_FAILURE;
    }

    cvx::CvxFont font(FONT_PATH);
    cv::Scalar textColor = convertColor(u32Color);

    for (auto &textCtx : textCtxs)
    {
        if (!textCtx.text.empty())
        {
            cvx::putText(bgrImg, textCtx.text, cv::Point(textCtx.x, textCtx.y), font, textCtx.fontSize, textColor);
        }
    }

    RK_MPI_SYS_MmzFlushCache(pstFrameInfo->stVFrame.pMbBlk, RK_FALSE);

    return RK_SUCCESS;
}

RK_S32 rkCvOsdEngText(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<OsdTextInfoCtx> textCtxs, RK_U32 u32Color)
{
    cv::Mat bgrImg;
    int width = pstFrameInfo->stVFrame.u32Width;
    int height = pstFrameInfo->stVFrame.u32Height;

    bgrImg = cv::Mat(height, width, CV_8UC3, pstFrameInfo->stVFrame.pVirAddr[0]);
    if (bgrImg.empty())
    {
        RK_LOGE("Failed to create cvImage.");
        return RK_FAILURE;
    }

    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    int thickness = 2;
    cv::Scalar textColor = convertColor(u32Color);

    for (auto &textCtx : textCtxs)
    {
        if (!textCtx.text.empty())
        {
            double fontScale = (double)textCtx.fontSize / 10;
            cv::putText(bgrImg, textCtx.text, cv::Point(textCtx.x, textCtx.y), fontFace, fontScale, textColor, thickness);
        }
    }

    RK_MPI_SYS_MmzFlushCache(pstFrameInfo->stVFrame.pMbBlk, RK_FALSE);

    return RK_SUCCESS;
}

RK_VOID rkCvOsdAzimuth(VIDEO_FRAME_INFO_S *pstFrameInfo, DetBox detBox, RK_U32 u32Color, RK_U32 u32Thick, RK_S32 s32CurAngle)
{
    cv::Mat bgrImg;
    int width = pstFrameInfo->stVFrame.u32Width;
    int height = pstFrameInfo->stVFrame.u32Height;

    bgrImg = cv::Mat(height, width, CV_8UC3, pstFrameInfo->stVFrame.pVirAddr[0]);
    if (bgrImg.empty())
    {
        RK_LOGE("Failed to create cvImage.");
        return;
    }

    int start_x = detBox.x;
    int end_x = detBox.x + detBox.width;
    int ruler_y = detBox.y + detBox.height / 2;
    int tick_length = 10;

    cv::Scalar color = convertColor(u32Color);

    // 绘制水平线
    cv::line(bgrImg, cv::Point(start_x, ruler_y), cv::Point(end_x, ruler_y), color, u32Thick);

    // 绘制刻度和标识
    for (int angle = -180; angle <= 180; angle += 10)
    {
        int x = start_x + (angle + 180) * detBox.width / 360;
        if (angle % 60 == 0 || angle == s32CurAngle)
        {
            cv::line(bgrImg, cv::Point(x, ruler_y - tick_length), cv::Point(x, ruler_y + tick_length), color, u32Thick);
            std::string text = std::to_string(angle);
            int baseline;
            cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
            cv::putText(bgrImg, text, cv::Point(x - textSize.width / 2, ruler_y + tick_length + textSize.height + 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, color, u32Thick);
        }
        else
        {
            cv::line(bgrImg, cv::Point(x, ruler_y - tick_length / 2), cv::Point(x, ruler_y + tick_length / 2), color, u32Thick);
        }
    }

    // 绘制当前方位角的指示
    if (s32CurAngle >= -180 && s32CurAngle <= 180)
    {
        int curAngle_x = start_x + (s32CurAngle + 180) * detBox.width / 360;

        std::vector<cv::Point> triPoints;
        triPoints.push_back(cv::Point(curAngle_x, ruler_y));
        triPoints.push_back(cv::Point(curAngle_x - 10, ruler_y - tick_length - 10));
        triPoints.push_back(cv::Point(curAngle_x + 10, ruler_y - tick_length - 10));

        cv::fillPoly(bgrImg, std::vector<std::vector<cv::Point>>{triPoints}, color);
    }

    RK_MPI_SYS_MmzFlushCache(pstFrameInfo->stVFrame.pMbBlk, RK_FALSE);

    return;
}

RK_VOID rkCvOsdPicth(VIDEO_FRAME_INFO_S *pstFrameInfo, DetBox detBox, RK_U32 u32Color, RK_U32 u32Thick, RK_S32 s32CurAngle)
{
    cv::Mat bgrImg;
    int width = pstFrameInfo->stVFrame.u32Width;
    int height = pstFrameInfo->stVFrame.u32Height;

    bgrImg = cv::Mat(height, width, CV_8UC3, pstFrameInfo->stVFrame.pVirAddr[0]);
    if (bgrImg.empty())
    {
        RK_LOGE("Failed to create cvImage.");
        return;
    }

    int start_y = detBox.y;
    int end_y = detBox.y + detBox.height;
    int ruler_x = detBox.x + detBox.width / 2;
    int tick_length = 10;

    cv::Scalar color = convertColor(u32Color);

    // 绘制水平线
    cv::line(bgrImg, cv::Point(ruler_x, start_y), cv::Point(ruler_x, end_y), color, u32Thick);

    // 绘制刻度和标识
    for (int angle = 90; angle >= -90; angle -= 10)
    {
        int y = start_y + (90 - angle) * detBox.height / 180;
        if (angle % 30 == 0 || angle == s32CurAngle)
        {
            cv::line(bgrImg, cv::Point(ruler_x - tick_length, y), cv::Point(ruler_x + tick_length, y), color, u32Thick);
            std::string text = std::to_string(angle);
            int baseline;
            cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
            cv::putText(bgrImg, text, cv::Point(ruler_x + tick_length + 5, y + textSize.height / 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, color, u32Thick);
        }
        else
        {
            cv::line(bgrImg, cv::Point(ruler_x - tick_length / 2, y), cv::Point(ruler_x + tick_length / 2, y), color, u32Thick);
        }
    }

    // 绘制当前方位角的指示
    if (s32CurAngle >= -90 && s32CurAngle <= 90)
    {
        int curAngle_y = start_y + (90 - s32CurAngle) * detBox.height / 180;

        std::vector<cv::Point> triPoints;
        triPoints.push_back(cv::Point(ruler_x, curAngle_y));
        triPoints.push_back(cv::Point(ruler_x - tick_length - 10, curAngle_y + 10));
        triPoints.push_back(cv::Point(ruler_x - tick_length - 10, curAngle_y - 10));

        cv::fillPoly(bgrImg, std::vector<std::vector<cv::Point>>{triPoints}, color);
    }

    RK_MPI_SYS_MmzFlushCache(pstFrameInfo->stVFrame.pMbBlk, RK_FALSE);

    return;
}

//RK_S32 rkText2BmpData(RGN_CFG *pstRgnCfg, void *pBmpData)
//{
//    if (SDL_Init(SDL_INIT_VIDEO) < 0)
//    {
//        RK_LOGE("Failed to initialize SDL: %s", SDL_GetError());
//        return RK_FAILURE;
//    }
//
//    if (TTF_Init() < 0)
//    {
//        RK_LOGE("Failed to initialize TTF: %s", TTF_GetError());
//        SDL_Quit();
//        return RK_FAILURE;
//    }
//
//    TTF_Font *font = TTF_OpenFont(FONT_PATH, pstRgnCfg->u32FontSize);
//    if (font == NULL)
//    {
//        RK_LOGE("Failed to load font: %s", SDL_GetError());
//        SDL_Quit();
//        return RK_FAILURE;
//    }
//
//    SDL_Color textColor = {0};
//    rkSetTextColor(&textColor, pstRgnCfg->font_color);
//
//    SDL_Surface *textSurface = TTF_RenderUTF8_Solid_Wrapped(font, pstRgnCfg->text.c_str(), textColor, pstRgnCfg->u32RgnWidth);
//    if (!textSurface)
//    {
//        RK_LOGE("Failed to render text: %s", SDL_GetError());
//        TTF_CloseFont(font);
//        TTF_Quit();
//        SDL_Quit();
//        return RK_FAILURE;
//    }
//
//    SDL_Surface *dstSurface = SDL_ConvertSurfaceFormat(textSurface, SDL_PIXELFORMAT_BGRA5551, 0);
//
//    memset(pBmpData, 0, pstRgnCfg->u32RgnWidth * pstRgnCfg->u32RgnHeight * 2);
//    memcpy(pBmpData, dstSurface->pixels, dstSurface->pitch * dstSurface->h);
//
//    SDL_FreeSurface(textSurface);
//    SDL_FreeSurface(dstSurface);
//    TTF_CloseFont(font);
//    TTF_Quit();
//    SDL_Quit();
//
//    return RK_SUCCESS;
//}