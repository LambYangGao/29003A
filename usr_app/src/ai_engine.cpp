#include "ai_engine.h"
#include "protocol_common.h"
AI_ENGINE::AI_ENGINE(void)
{
    cfg = new AiEngineCfg();
    detector = new RK3588Detector();
    motTracker = new MotCTracker();
}

AI_ENGINE::~AI_ENGINE(void)
{
    if (detector)
    {
        delete detector;
        detector = nullptr;
    }
    if (motTracker)
    {
        delete motTracker;
        motTracker = nullptr;
    }
    if (cfg)
    {
        delete cfg;
        cfg = nullptr;
    }
}

void AI_ENGINE::init(void)
{
	detectParams.model_path = cfg->detect_model_path;
	detectParams.anchor_path = "";
	detectParams.algo_type = "yolov8";
	detectParams.platform = DEVICE_PLATFORM::RK3588;
	detectParams.device_id = 0;
	detector->init(detectParams);

    motTracker->init(cfg->mot_class_num, cfg->mot_queue_num, cfg->mot_loss_num, cfg->mot_iou_thresh);
}

void AI_ENGINE::run(void)
{
    RK_S32 s32Ret = RK_SUCCESS;
    VIDEO_FRAME_INFO_S inferFrame; //图像帧 from(VPSS通道)
    memset(&inferFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
    cv::Mat image;//MAT图像 from(图像帧) to(检测器)
    std::vector<std::vector<ObjectInfo>> curTrackObjs;//多目标跟踪结果 
    while (!cfg->bThreadExit)
    {
        if (!cfg->b_detect_on)
        {
            if (targetDetectRes_Msg.size() > 0)
            {
                trackObjs.clear();
                targetDetectRes_Msg.clear();
            }
            usleep(100000);
        }
        else
        {
        // 目标检测
            // 取图
            s32Ret = RK_MPI_VPSS_GetChnFrame(cfg->u32VpssGroup, cfg->u32VpssChn, &inferFrame, cfg->s32TimeOutMs);
            if (s32Ret != RK_SUCCESS)
            {
                RK_LOGE("RK_MPI_VPSS_GetChnFrame failed!");
                //RK_MPI_VPSS_ReleaseChnFrame(cfg->u32VpssGroup, cfg->u32VpssChn, &inferFrame);
                continue;
            }
            image = cv::Mat(inferFrame.stVFrame.u32Height, inferFrame.stVFrame.u32VirWidth, CV_8UC3, inferFrame.stVFrame.pVirAddr[0]);
            //检测动作
            results.clear();
            int32_t lastTime = getCurTickCount();
            detector->detect_cvmat(image, cfg->confThesh, cfg->nmsThresh, results);
			int32_t curTime = getCurTickCount();
			RK_LOGE("detect time :%d", curTime - lastTime);
            RK_LOGE("detect res num:%d", results.size());
            //多目标跟踪
            std::vector<TargetDetectRes_A2B_MSG> curResMsg;  // 目标检测结果
            if (cfg->b_mot_on)//b_mot_on
            {
                curTrackObjs.clear();
                motTracker->track(results);
                curTrackObjs = motTracker->getHistoryObjects();
               
                //跟踪结果处理
                uint16_t targetNums = 0;
                for (int i = 0; i < curTrackObjs.size(); i++)
                {
                    for (int j = 0; j < curTrackObjs[i].size(); j++)
                    {
                        //根据当前主视场，坐标归一化
                        mapCoordToOriginal(curTrackObjs[i][j].bbox, cfg->u32PicHeight, cfg->u32PicWidth, cfg->u32OriHeight, cfg->u32OriWidth);
                        //点迹质量管理-是否起批
                        if (curTrackObjs[i][j].match_quality > 3)
                        {
							curTrackObjs[i][j].isUpload = true;
							targetNums++;
                        }
                        else
                        {
                            curTrackObjs[i][j].isUpload = false;
                        }
                        //统计结果for send to 3588B
                        if (curTrackObjs[i][j].isUpload)
                        {
                            
                            TargetDetectRes_A2B_MSG a2bMsg;
                            a2bMsg.HEAD1 = 0x80;
                            a2bMsg.HEAD2 = 0x02;
                            a2bMsg.d_num = targetNums;
                            a2bMsg.bbox = curTrackObjs[i][j].bbox;
                            a2bMsg.f_score = curTrackObjs[i][j].class_score;
                            //目标类别 0船，1浮标，2人，3车，4建筑，5坦克
                            a2bMsg.d_Target_Category = i;
							curResMsg.push_back(a2bMsg);
                        }
                        
                    }
                }
                mtx.lock();
                trackObjs.clear();
                trackObjs.swap(curTrackObjs);
                targetDetectRes_Msg.clear();
                targetDetectRes_Msg.swap(curResMsg);
                mtx.unlock();

                int resSize = 0;
                for (int i = 0; i < trackObjs.size(); i++)
                {
                    resSize += trackObjs[i].size();   
                }
                RK_LOGE("mot res num:%d", resSize);
 
                // 序列化vector
                size_t dataSize = targetDetectRes_Msg.size() * sizeof(TargetDetectRes_A2B_MSG);
                boardabcMsg->Send_A_to_B(targetDetectRes_Msg.data(), dataSize);
            }
            // 释放帧数据


            s32Ret = RK_MPI_VPSS_ReleaseChnFrame(cfg->u32VpssGroup, cfg->u32VpssChn, &inferFrame);
            if (s32Ret != RK_SUCCESS)
            {
                RK_LOGE("RK_MPI_VPSS_ReleaseChnFrame failed!");
            }

            
        }
    }

    RK_LOGD("%s out\n", __FUNCTION__);
}

void AI_ENGINE::letterbox(const cv::Mat &originImage, const cv::Size& inpSize, cv::Mat& paddingImage)
{
        //cv::cvtColor(originImage, originImage, cv::COLOR_BGR2RGB);
        int paddingImageH = inpSize.height;
        int paddingImageW = inpSize.width;
        int originImageH = originImage.rows;
        int originImageW = originImage.cols;
        float ratioH = static_cast<float>(paddingImageH) / originImageH;
        float ratioW = static_cast<float>(paddingImageW) / originImageW;

        float r = std::min(ratioH, ratioW);
        int resizedH = std::ceil(originImageH * r);
        int resizedW = std::ceil(originImageW * r);

        cv::Mat resizedImage;
        cv::resize(originImage, resizedImage, cv::Size(resizedW, resizedH));

        double dw = paddingImageW - resizedW;
        dw /= 2;
        double dh = paddingImageH - resizedH;
        dh /= 2;

        double top, bottom, left, right;
        top = round(dh - 0.1);
        bottom = round(dh + 0.1);
        left = round(dw - 0.1);
        right = round(dw + 0.1);
        cv::copyMakeBorder(resizedImage, paddingImage, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));    
}

void AI_ENGINE::mapCoordToOriginal(cv::Rect& bbox, int cur_height, int cur_width, int ori_height, int ori_width)
{
    float ww = (float)ori_width / (float)cur_width;
    float hh = (float)ori_height / (float)cur_height;
    bbox.x = (float)bbox.x * ww;
    bbox.y = (float)bbox.y * hh;
    bbox.width = (float)bbox.width * ww;
    bbox.height = (float)bbox.height * hh;
}

