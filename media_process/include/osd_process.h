#ifndef _OSD_PROCESS_H_
#define _OSD_PROCESS_H_

#include "rkmodel_build.h"
#include "rk_mpi_vgs.h"
#include "rk_mpi_cal.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

//#include "SDL.h"
//#include "SDL_ttf.h"

#include "cvxFont.h"

#define FONT_PATH "resource/fonts/mshybd.ttc"

struct PointXY
{
    int x;
    int y;
};

struct DetBox
{
    int x;
    int y;
    int width;
    int height;
};

struct OsdTextInfoCtx
{
    int x;
    int y;
    int fontSize;
    std::string text;
};
//十字detBox x，y是中心；
//目标框x，y是左上角
RK_S32 rkVgsOsdFrame(VIDEO_FRAME_INFO_S *pOuterFrame, VIDEO_FRAME_INFO_S *pInnerFrame, PointXY point);

RK_S32 rkVgsRealRect(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick);

RK_S32 rkVgsFalseRect(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick);

RK_S32 rkVgsRealCross(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick);

RK_S32 rkVgsFalseCross(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick);

RK_S32 rkCvRealRect(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick);

RK_S32 rkCvFalseRect(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick);

RK_S32 rkCvRealCross(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick);

RK_S32 rkCvFalseCross(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<DetBox> detBoxs, RK_U32 u32Color, RK_U32 u32Thick);

RK_S32 rkCvOsdText(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<OsdTextInfoCtx> textCtxs, RK_U32 u32Color);

RK_S32 rkCvOsdEngText(VIDEO_FRAME_INFO_S *pstFrameInfo, const std::vector<OsdTextInfoCtx> textCtxs, RK_U32 u32Color);

RK_VOID rkCvOsdAzimuth(VIDEO_FRAME_INFO_S *pstFrameInfo, DetBox detBox, RK_U32 u32Color, RK_U32 u32Thick, RK_S32 s32CurAngle);

RK_VOID rkCvOsdPicth(VIDEO_FRAME_INFO_S *pstFrameInfo, DetBox detBox, RK_U32 u32Color, RK_U32 u32Thick, RK_S32 s32CurAngle);

//RK_S32 rkText2BmpData(RGN_CFG *pstRgnCfg, void *pBmpData);

#endif // _OSD_PROCESS_H_