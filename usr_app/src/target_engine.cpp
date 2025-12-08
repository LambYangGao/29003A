#include "target_engine.h"
#include <rk_debug.h>
#include <ctime>
#include<time.h>
#include <chrono>

#include  "protocol_board_abc.h"
#include  "protocol_jgzm.h"
#include  "protocol_common.h"
#include  "protocol_sf.h"
#include  "xkMsg.h"

using namespace std::chrono;

extern GlobalReatimeMsg* globalMsg;  //全局状态信息  包含相机、激光测距、伺服
extern SF_msg* sf_msg;                     //伺服消息处理对象
extern BOARDABC_MSG* boardabcMsg;//RK网口通信
extern Jgzm_msg* jgzmmsg;

TARGET_ENGINE::TARGET_ENGINE()
{
    cfg = new TargetEngineCfg();
    trackRes = new TargetTrackRes();

    //目标跟踪
    tracker = NULL;
    //目标定位
    targetLoc = new TargetLoclization();

}

TARGET_ENGINE::~TARGET_ENGINE()
{
    if (trackRes)
    {
        delete trackRes;
        trackRes = NULL;
    }

    if (tracker != nullptr)
    {
        delete tracker;
        tracker = nullptr;
    }
    if (targetLoc != nullptr)
    {
        delete targetLoc;
        targetLoc = nullptr;
    }

    if (cfg)
    {
        delete cfg;
        cfg = nullptr;
    }

}

void TARGET_ENGINE::init()
{
    cfg->setMainView(1);
}

void TARGET_ENGINE::run()
{
    RK_S32 s32Ret = RK_SUCCESS;
    VIDEO_FRAME_INFO_S inferFrame; //图像帧 from(VPSS通道)
    memset(&inferFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
    cv::Mat image;//MAT图像 from(图像帧) to(跟踪器)
    bool first = true;//是否首次跟踪 (首次赋波门)
    int memoryCount = 0;//记忆跟踪计数
    POINT_S track_point;//目标中心点
    TargetTrackRes* curTrackRes = new TargetTrackRes();//目标跟踪结果
    int32_t lastTime = getCurTickCount();
    time_t time_tlase = time(NULL);;
    clock_t vlastTime = clock();
    int32_t curTime;
    int32_t frame = 0;
    time_t now;
    int ret = 2;
    while (!cfg->bThreadExit)
    {
        if (!cfg->b_sot_on)
        {
            //跟踪退出需要删除实例并重新实例化
            if (tracker != nullptr)
            {
                RK_LOGE("tracker != nullptr,tracker clean");
                delete tracker;
                tracker = nullptr;
                first = true;
				curTrackRes->trackMode = 0;
				curTrackRes->predBox.x = 0;
				curTrackRes->predBox.y = 0;
				curTrackRes->predBox.width = 0;
				curTrackRes->predBox.height = 0;
                trackOff();
                RK_LOGE("tracker != nullptr,tracker clean success");
            }
            usleep(100000);
            continue;
        }
        else
        {
            //计算跟踪帧频
            //if (getCurTickCount() - lastTime > 1000)
            //{
            //    lastTime = getCurTickCount();
            //    RK_LOGE("track frame: %d\n", frame);
            //    frame = 0;
            //}
            //else
            //{
            //    frame++;
            //}

            curTrackRes->trackMode = 1;
            //目标跟踪
            //如果时自动跟踪模式 取图算脱靶量
            if (globalMsg->m_systemControlMsgS.WorkMode == SYS_MODE_AUTO_TRACK)
            {
                // 取图
                s32Ret = RK_MPI_VPSS_GetChnFrame(cfg->u32VpssGroup, cfg->u32VpssChn, &inferFrame, cfg->s32TimeOutMs);
                if (s32Ret != RK_SUCCESS)
                {
                    RK_LOGE("RK_MPI_VPSS_GetChnFrame failed!");
                    //RK_MPI_VPSS_ReleaseChnFrame(cfg->u32VpssGroup, cfg->u32VpssChn, &inferFrame);
                    continue;
                }
                image = cv::Mat(inferFrame.stVFrame.u32Height, inferFrame.stVFrame.u32VirWidth, CV_8UC3, inferFrame.stVFrame.pVirAddr[0]);

                //跟踪
                //lastTime = getCurTickCount();
                if (first)
                {
                    RK_LOGE("track start\r\n");
                    tracker = new JZSoTTracker();
                    tracker->disableLogInfo();
                    //tracker->enableLogInfo();
                    tracker->setDisThresh(cfg->dissapearThesh);

                    if (m_is_external_init)
                    {
                        tracker->init(cfg->initBox_extern, image);
                    }
                    else
                    {
                        tracker->init(cfg->initBox, image);
                    }
 
                    //RK_LOGE("initBox x:%f y:%f w:%f h:%f", cfg->initBox.x, cfg->initBox.y, cfg->initBox.width, cfg->initBox.height);
                    first = false;
                    //cv::imwrite("tmp_track_init.bmp", image);
                }

                ret = tracker->update(image, curTrackRes->predBox);
                curTrackRes->trackState = ret;
                //RK_LOGE("predBox ret:%d", ret);
            }   
            //RK_LOGE("track update %d\n", getCurTickCount() - lastTime);
            //计算脱靶量
            if ((0 == ret) || (-2 == ret))
            {
                if (-2 == ret)
                {
					         
                    memoryCount++;
                }
                else
                {
                    memoryCount = 0;
                }

                if (memoryCount > 1500)
                {
                    printf("Memory track failed.\n");
                    curTrackRes->trackMode = 0;
                    trackOff();
                }
                else
                {
                   // RK_LOGE("predBox x:%f y:%f w:%f h:%f", curTrackRes->predBox.x, curTrackRes->predBox.y, curTrackRes->predBox.width, curTrackRes->predBox.height);
                    track_point.s32X = (RK_S32)(curTrackRes->predBox.x + curTrackRes->predBox.width / 2.f);
                    track_point.s32Y = (RK_S32)(curTrackRes->predBox.y + curTrackRes->predBox.height / 2.f);
                    //RK_LOGE("s32X:%d s32Y:%d", track_point.s32X, track_point.s32Y);
                    curTrackRes->target_x = track_point.s32X - cfg->u32OriWidth/2;
                    curTrackRes->target_y = track_point.s32Y - cfg->u32OriHeight/2;  
                    float fov_angle_azi, fov_angle_pitch;
                    if (cfg->u32MainView == 1)
                    {
                        fov_angle_azi = globalMsg->m_camVIMsg.f_VIView_Azi;
                        fov_angle_pitch = globalMsg->m_camVIMsg.f_VIView_Pitch;
                    }
                    else if (cfg->u32MainView == 2)
                    {
                        fov_angle_azi = globalMsg->m_camIRMsg.f_IRZView_Azi;
                        fov_angle_pitch = globalMsg->m_camIRMsg.f_IRZView_Pitch;
                    }
                   // RK_LOGE("fov_angle_azi:%f fov_angle_pitch:%f,y:u32OriWidth:%d,u32OriHeight:%d", fov_angle_azi, fov_angle_pitch, cfg->u32OriWidth, cfg->u32OriHeight);
                    curTrackRes->target_fw = (float)(curTrackRes->target_x) * fov_angle_azi / (float)cfg->u32OriWidth;
                    curTrackRes->target_fy = (-1.0) * (float)(curTrackRes->target_y) * fov_angle_pitch / (float)cfg->u32OriHeight;//
					int sfMainView;
					switch (globalMsg->m_picCtlMsg.e_Switch_MainScreen)
					{
					case 1:
						sfMainView = 1;
						break;
					case 2:
						sfMainView = 2;
						break;
					default:
						break;
					}
                    if (0 == ret)
                    {
                        if (1 == curTrackRes->trackMode)
                        {
                            //控制伺服-脱靶量 速度 
                            //RK_LOGE("e_Switch_MainScreen: %d,sfMainView: %d", g_fcae_msg->g_PicControlMsgS.e_Switch_MainScreen, sfMainView);
                            RK_LOGE("track res:x: %d,y: %d,fw:%f.fy:%f", curTrackRes->target_x, curTrackRes->target_y, curTrackRes->target_fw, curTrackRes->target_fy);
                            tracker->setDisThresh(cfg->dissapearThesh);
                            sf_msg->SendTrack(curTrackRes->target_fw, curTrackRes->target_fy, 10);
                        }
                    }
                    else if (-2 == ret)
                    {
                        if (1 == curTrackRes->trackMode)
                        {
                            tracker->setDisThresh(cfg->dissapearThesh + 0.05);

                            //计算预推脱靶量
                            sf_msg->SendMemTrack();
                        }
                    }
                }
            }
            else if (-1 == ret)
            {
                printf("Track failed.\n");
                curTrackRes->trackMode = 0;
                trackOff();
            }    

            // 释放帧数据
            s32Ret = RK_MPI_VPSS_ReleaseChnFrame(cfg->u32VpssGroup, cfg->u32VpssChn, &inferFrame);
            if (s32Ret != RK_SUCCESS)
            {
                RK_LOGE("RK_MPI_VPSS_GetChnFrame failed!");
            }


            //目标定位-测距值-被动定位
            if (cfg->b_location_on)
            {
               
                //RK_LOGE("b_location_on:%d", cfg->b_location_on);
                double distence;
                double fw_sf, fw_gd;
                fw_sf = globalMsg->m_sfMsg.f_Azi_value;                //逆时针0~360为正值
                fw_gd = globalMsg->m_gnssMsg.d_Heading_value;  //到时候具体看惯导，默认是逆时针0~180为正，顺时针0~180为负值
                //若激光测距开且有主波有回波，根据激光测距值，惯导经纬高，伺服方位俯仰，计算目标经纬高
				if (jgzmmsg->m_bhaveDisValue &&!globalMsg->m_laserStateMsg.uc_lockStatus&& globalMsg->m_laserStateMsg.uc_powerState)//激光测距开启 有值
                {
                    distence = globalMsg->m_laserStateMsg.f_Dis_value;
					//函数输入方位为逆时针0~360，需要把-180~180转换为0~360
                    //RK_LOGE("JD:%f WD:%f GD:%f",
                    //    g_gd_msg->m_gd_recv_msg_value.LONGITUDE_Value,
                    //    g_gd_msg->m_gd_recv_msg_value.LATITUDE_Value,
                    //    g_gd_msg->m_gd_recv_msg_value.HEIGHT_Value);
					targetLoc->targetGpsFromPolar(fw_sf,
						globalMsg->m_sfMsg.f_Pitch_value,
						distence,
						fw_gd,
						globalMsg->m_gnssMsg.d_Pitch_value,
						globalMsg->m_gnssMsg.d_Roll_value,
						globalMsg->m_gnssMsg.d_GPS_Longitude_value,
						globalMsg->m_gnssMsg.d_GPS_Latitude_value,
						globalMsg->m_gnssMsg.d_GPS_Altitude_value,
						curTrackRes->target_jd,
						curTrackRes->target_wd,
						curTrackRes->target_gd,
						SFAngleMode::AIR_GD
					);

					//RK_LOGE("curTrackRes->target_jd;%f", curTrackRes->target_jd);
					//RK_LOGE("curTrackRes->target_wd;%f", curTrackRes->target_wd);
					//RK_LOGE("curTrackRes->target_gd;%f", curTrackRes->target_gd);

					curTrackRes->target_distance = distence;
                }
                else//模拟距离
                {
                    if (globalMsg->m_laserStateMsg.b_simuDis)
                    {
						//模拟距离
						distence = 1500;
						fw_gd = 0;
						globalMsg->m_gnssMsg.d_Pitch_value = 0;
						globalMsg->m_gnssMsg.d_Roll_value = 0;
						globalMsg->m_gnssMsg.d_GPS_Longitude_value = 103.4;
						globalMsg->m_gnssMsg.d_GPS_Latitude_value = 32.5;
						globalMsg->m_gnssMsg.d_GPS_Altitude_value = 526;

						targetLoc->targetGpsFromPolar(fw_sf,
							globalMsg->m_sfMsg.f_Pitch_value,
							distence,
							fw_gd,
							globalMsg->m_gnssMsg.d_Pitch_value,
							globalMsg->m_gnssMsg.d_Roll_value,
							globalMsg->m_gnssMsg.d_GPS_Longitude_value,
							globalMsg->m_gnssMsg.d_GPS_Latitude_value,
							globalMsg->m_gnssMsg.d_GPS_Altitude_value,
							curTrackRes->target_jd,
							curTrackRes->target_wd,
							curTrackRes->target_gd,
							SFAngleMode::AIR_GD
						);
                    }
                }
            }
        }

        mtx.lock();
        memcpy(trackRes, curTrackRes, sizeof(TargetTrackRes));
        mtx.unlock();

        //同步检测结果给B板
        trackInfoSycToB();
    }

    RK_LOGD("%s out\n", __FUNCTION__);
}

void TARGET_ENGINE::trackOff()
{
    RK_LOGE("trackOff");
    cfg->b_sot_on = RK_FALSE;//跟踪关
    trackRes->trackMode = 0;
    if (globalMsg->m_systemControlMsgS.WorkMode == SYS_MODE_AUTO_TRACK)
    {
        printf("track off\n");
        globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_MANUAL;//系统工作模式
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        //控制伺服退出跟踪，锁定当前方位俯仰
        for (int i = 0; i < 2; i++)
        {
            printf("sf sudu 0\n");
            sf_msg->SendFollow(globalMsg->m_sfMsg.f_Azi_value, globalMsg->m_sfMsg.f_Pitch_value);
        }
    }
}
void TARGET_ENGINE::trackOn(cv::Rect& outInitBox)
{
	cfg->b_sot_on = RK_TRUE;
    globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_AUTO_TRACK;
    syncGlobalToBoardLan();
    boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
    if (outInitBox.x == 0 || outInitBox.y == 0 ||
        outInitBox.width == 0 || outInitBox.height == 0)
    {
        m_is_external_init = false;
        cfg->initBox_extern = cv::Rect(0, 0, 0, 0);
    }
    else
    {
        m_is_external_init = true;
        cfg->initBox_extern = outInitBox;
    }
    
	//存储压缩视频Init_RK3588A_TO_B_MSG
	//属性识别 SEND 3588B
	//g_rk3588abc_msg->Init_RK3588A_TO_B_MSG();
	//g_rk3588abc_msg->rk3588A_to_B_Msg->AI_ONOFF = 1;
	//RK_LOGE("AI_ONOFF:%d", g_rk3588abc_msg->rk3588A_to_B_Msg->AI_ONOFF);
	//g_rk3588abc_msg->Send_A_to_B((void*)g_rk3588abc_msg->rk3588A_to_B_Msg, sizeof(RK3588A_TO_B_MSG));
}

void TARGET_ENGINE::trackInfoSycToB()
{
	boardabcMsg->targetTrackRes_Msg->predBox = cv::Rect(trackRes->predBox.x, trackRes->predBox.y,
		trackRes->predBox.width, trackRes->predBox.height);
	boardabcMsg->targetTrackRes_Msg->target_distance = trackRes->target_distance;
	boardabcMsg->targetTrackRes_Msg->trackMode = trackRes->trackMode;
    boardabcMsg->targetTrackRes_Msg->trackState = trackRes->trackState;
	boardabcMsg->targetTrackRes_Msg->target_x = trackRes->target_x;
	boardabcMsg->targetTrackRes_Msg->target_y= trackRes->target_y;
    boardabcMsg->targetTrackRes_Msg->target_fw = trackRes->target_fw;
    boardabcMsg->targetTrackRes_Msg->target_fy = trackRes->target_fy;
    boardabcMsg->targetTrackRes_Msg->target_jd = trackRes->target_jd;
    boardabcMsg->targetTrackRes_Msg->target_wd = trackRes->target_wd;
    boardabcMsg->targetTrackRes_Msg->target_gd = trackRes->target_gd;
    boardabcMsg->targetTrackRes_Msg->target_ve = trackRes->target_ve;
    boardabcMsg->targetTrackRes_Msg->target_vn = trackRes->target_vn;
    boardabcMsg->targetTrackRes_Msg->target_vu = trackRes->target_vu;
    boardabcMsg->Send_A_to_B((void*)boardabcMsg->targetTrackRes_Msg, sizeof(TargetTrackRes_A2B_MSG));
}




