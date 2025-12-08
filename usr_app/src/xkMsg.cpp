#include "xkMsg.h"
#include "ConfigManager.h"
#include "protocol_ir.h"
#include "protocol_vi.h"
#include "protocol_jgzm.h"
#include "video_osd.h"
#include "vpss_engine.h"
#include "protocol_board_abc.h"
#include "target_engine.h"
#include "protocol_power.h"
#include "protocol_sf.h"        
#include "ai_engine.h"          
#include "geographic_track.h"
#include "protocol_picxk.h"

static bool validate_range(int32_t v, int32_t lo, int32_t hi, const char* name)
{
    if (v < lo || v > hi)
    {
        RK_LOGE("XK参数 %s 超出范围 [%d,%d], 实际值 %d", name, lo, hi, v);
        return false;
    }
    return true;
}

extern ConfigManager* g_configManager;
extern POWER_msg* power_msg;
extern Vi_msg* ccdmsg;
extern IR_msg* irmsg;
extern Jgzm_msg* jgzmmsg;
extern VPSS_ENGINE* g_vpss_engine;
extern GlobalReatimeMsg* globalMsg;
extern VIDEO_OSD* g_video_osd;
extern TARGET_ENGINE* target_engine;
extern BOARDABC_MSG* boardabcMsg;
extern SF_msg* sf_msg;
extern AI_ENGINE* ai_engine;
extern GeoGraphicTrack* geo_track;

/*  uint8_t HEAD1 = 0x80;
    uint8_t HEAD2 = 0x01;
    uint8_t WorkMode;         //工作模式 0-无效 1-手动控制 2-自动跟踪 3-无人值守 4-前置模式
    uint8_t MainView;          //选择主视图通道 1-可见光 2-红外
    uint8_t PicControl_PIP;   //画中画控制 1-打开 2-关闭 3-画中画图像切换 4-左半屏显示 5-右半屏显示 6-画中画位置顺时针切换
    uint8_t OSD_LineColor;   //瞄准线颜色 1-红色 2-绿色 3-蓝色 4-白色
    uint8_t OSD_WordDis;     //文字显示 1-文字显示1级 2-文字显示2级 3-文字显示3级
    uint8_t OSD_Level;          //OSD显示等级 0 1-全显 2-简化显示 3-全隐

    uint8_t AI_ONOFF;         //AI检测开关 1-开启 2-关闭
    uint8_t TARGET_ONOFF; //目标跟踪开关 1-开启 2-关闭
    uint8_t AI_detectLevel;   //AI检测灵敏度 1-低 2-高
    uint8_t Stitching_ONOFF; //拼接开关 0-无效 1-开启 2-关闭
    uint8_t Fusion_Flag;        //图像融合标志 0-无效 1-可见光融合 2-可见光+SAR 3-红外+SAR 4-关闭
    uint8_t VisualLand_ONOFF;//视觉着陆开关 0-无效 1-开启 2-关闭
    uint8_t JXPP_ONOFF;        //景象匹配开关 0-无效 1-开启 2-关闭
    uint8_t Data1;//数据1
    uint8_t Data2;//数据2*/
void syncGlobalToBoardLan()
{
    boardabcMsg->boardA_to_BC_Msg->WorkMode = globalMsg->m_systemControlMsgS.WorkMode;
    boardabcMsg->boardA_to_BC_Msg->MainView = globalMsg->m_mainViewState.MainView;
    boardabcMsg->boardA_to_BC_Msg->OSD_LineColor = globalMsg->m_picCtlMsg.e_OSD_LineColor;
    boardabcMsg->boardA_to_BC_Msg->OSD_WordDis = globalMsg->m_picCtlMsg.e_OSD_WordDis;
    boardabcMsg->boardA_to_BC_Msg->OSD_Level = globalMsg->m_picCtlMsg.e_OSD_Level;

    boardabcMsg->boardA_to_BC_Msg->AI_ONOFF = globalMsg->m_algoControlMsg.e_AI_ONOFF;
    boardabcMsg->boardA_to_BC_Msg->TARGET_ONOFF = globalMsg->m_algoControlMsg.e_TARGET_ONOFF;
    RK_LOGE("globalMsg->m_mainViewState.MainView : %d", globalMsg->m_mainViewState.MainView);
}

XKMsg::XKMsg()
{
    m_xkDownMsg = new XKDownMsg();
    m_picupRealTimeStateMsgStruct = new PICUpRealTimeStateMsgStruct();
    m_picupSelfCheckStateStruct = new PICUpSelfCheckStateStruct();

    initSelfCheckStateStruct();
}

XKMsg::~XKMsg()
{
    if (m_xkDownMsg) {
        delete m_xkDownMsg;
        m_xkDownMsg = NULL;
    }
}

void XKMsg::initSelfCheckStateStruct()
{
    m_picupSelfCheckStateStruct->e_SocA_state = 0;      // 默认正常
    m_picupSelfCheckStateStruct->e_SocB_state = 0;
    m_picupSelfCheckStateStruct->e_Laser_state = 1;
    m_picupSelfCheckStateStruct->e_CCD_state = 1;
    m_picupSelfCheckStateStruct->e_IR_state = 1;
    m_picupSelfCheckStateStruct->e_SF_state = 1;
    m_picupSelfCheckStateStruct->e_Power_state = 1;
    m_picupSelfCheckStateStruct->temperature = 37.00;
    m_picupSelfCheckStateStruct->proc_voltage = 28.12;
}


int XKMsg::init_xk_socket()
{
    m_socket_xk = iSocket::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int ret = iSocket::bind(m_socket_xk, g_configManager->m_gd_configinfo->GD_Send_MNXK_Port);
    if (-1 == ret) {
        perror("初始化失败! ");
    }
    return 0;
}

void XKMsg::processCCD(XKDownMsg* _xkDownMsg)
{
    switch (_xkDownMsg->msg_type)
    {
    case E_FK_VL_ZOOM_INCREASES:  //变焦+
        ccdmsg->zoomTele();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_ZOOM_DECREASES: //变焦-
        ccdmsg->zoomWide();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_FOCUSING_INCREASES: //对焦+
        ccdmsg->focusFar();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_FOCUSING_DECREASES: //对焦-
        ccdmsg->focusNear();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_STOP_ZOOM:    //变焦/对焦停止
        ccdmsg->zoomStop();
        ccdmsg->focusStop();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_DEHAZE_OPEN:  //去雾开启
        ccdmsg->setElectronicDefogOn();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_DEHAZE_CLOSE: //去雾关闭
        ccdmsg->setElectronicDefogOff();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_IR_OPEN:
        ccdmsg->setFilterNearInfrared();   //可见光红外滤镜开启
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_IR_CLOSE:
        ccdmsg->setFilterVisible();  //可见光红外滤镜关闭
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VI_FOCUSE_AUTO:   //对焦自动
        ccdmsg->focusOnePush();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VI_FOCUSE_MANUL:  //对焦手动
        // 手动模式已通过具体指令控制，直接响应
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VI_FOCUSE_SEMIAUTO: //对焦半自动
        sendAck(_xkDownMsg->msg_type, XK_ERR_UNSUPPORTED);
        break;
    case E_FK_VL_RotationHorizontalOn: //水平旋转开启
        ccdmsg->setPictureFlipH();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_Vl_RotationHorizontalOff:   //水平旋转关闭
        ccdmsg->setPictureFlipNormal();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VL_RotationVerticallyOn: //垂直旋转开启
        ccdmsg->setPictureFlipV();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_Vl_RotationVerticallyOff: //垂直旋转关闭
        ccdmsg->setPictureFlipNormal();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_VI_Brightness:        //亮度调节
    {
        uint32_t level = _xkDownMsg->param_1;
        if (!validate_range(level, 0, 240, "VI增益")) {
            sendAck(_xkDownMsg->msg_type, XK_ERR_INVALID_ARG);
            break;
        }
        ccdmsg->setGainValue(static_cast<uint8_t>(level));
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    }
    default:
        break;
    }
}

void XKMsg::processIR(XKDownMsg* _xkDownMsg)
{
    switch (_xkDownMsg->msg_type)
    {
    case E_FK_IR_ZOOM_INCREASES:  //变焦+
        irmsg->SetZoom(true);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_ZOOM_DECREASES:  //变焦-
        irmsg->SetZoom(false);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_STOP_ZOOM:  //变焦停止
        irmsg->StopFocusZoom();
        irmsg->QueryPosition(); // 查询镜头位置/状态
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_FOCUSING_INCREASES:  //对焦+
        irmsg->SetFocus(true);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_FOCUSING_DECREASES:  //对焦-
        irmsg->SetFocus(false);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_BG_CORRECTION:           //背景校正
        irmsg->ManualBGCorrect();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_IMAGE_ENHANCEMENT_1:     //图像增强1档 0x64=100
        irmsg->SetImageEnhance(true);
        irmsg->SetEnhanceCoef(0x64);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_IMAGE_ENHANCEMENT_2:      //图像增强2档 0x80=128
        irmsg->SetImageEnhance(true);
        irmsg->SetEnhanceCoef(0x80);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_IMAGE_ENHANCEMENT_3:      //图像增强3档 0xC0=192
        irmsg->SetImageEnhance(true);
        irmsg->SetEnhanceCoef(0xC0);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_IMAGE_ENHANCEMENT_CLOSE:      //图像增强关闭
        irmsg->SetImageEnhance(false);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_AUTO_FOCUSING:                  //触发一次自动对焦
        irmsg->TriggerAF();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_OSD_CLOSE:      //十字线关闭
        irmsg->SetCrosshair(false);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_OSD_OPEN:      //十字线开启
        irmsg->SetCrosshair(true);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_IR_SEND_CUSTOMDATA:      //自定义指令
        sendAck(_xkDownMsg->msg_type, XK_ERR_UNSUPPORTED);
        break;
    default:
        break;
    }
}

void XKMsg::processJGCZ(XKDownMsg* _xkDownMsg)
{
    switch (_xkDownMsg->msg_type)
    {
    case E_FK_LASER_RANGING_STOP:
        jgzmmsg->sendStopCmd();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_LASER_RANGING_ONE:
        jgzmmsg->sendSingleRangingCmd();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    case E_FK_LASER_RANGING_MANY:
        jgzmmsg->sendContRangingCmd(5, 30000);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    default:
        break;
    }
}

void XKMsg::processSF(XKDownMsg* _xkDownMsg)
{
    switch (_xkDownMsg->msg_type)
    {
    case E_FK_SF_SHOUC:
    {
        sf_msg->SendCollect();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    }
    case E_FK_SF_STOP:
    {
        sf_msg->SendStop();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    }
    case E_FK_SF_RESTORE:
    {
        sf_msg->SendZero();
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    }
    case E_FK_SF_MOVE_TO:
    {
        int32_t azimuth = _xkDownMsg->param_1;
        int32_t pitch = _xkDownMsg->param_2;
        sf_msg->SendFollow(azimuth, pitch);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    }
    case E_FK_SF_FWAUTO:
    {
        int16_t centerAzi = _xkDownMsg->param_2;
        uint16_t scanRange = _xkDownMsg->param_3;
        uint16_t scanSpeed = _xkDownMsg->param_1;
        sf_msg->SendScan(centerAzi, scanRange, scanSpeed);
        sendAck(_xkDownMsg->msg_type, XK_ERR_OK);
        break;
    }
    default:
        break;
    }
}

void XKMsg::processAlgo(XKDownMsg* _xkDownMsg)
{
    switch (_xkDownMsg->msg_type)
    {
    case E_FK_AI_DETECT_OPEN:    //开启AI检测
    {
        RK_LOGE("E_FK_AI_DETECT_OPEN");

        globalMsg->m_algoControlMsg.e_AI_ONOFF = 1;
        ai_engine->cfg->b_detect_on = RK_TRUE;
        ai_engine->cfg->b_mot_on = RK_TRUE;
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    case E_FK_AI_DETECT_CLOSE:  //关闭AI检测
    {
        RK_LOGE("E_FK_AI_DETECT_CLOSE");

        globalMsg->m_algoControlMsg.e_AI_ONOFF = 2;
        ai_engine->cfg->b_detect_on = RK_FALSE;
        ai_engine->cfg->b_mot_on = RK_FALSE;
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    case E_FK_TK_SINGLE_OPEN:  //开启单目标跟踪
    {
        RK_LOGE("E_FK_TK_SINGLE_OPEN");

        globalMsg->m_algoControlMsg.e_TARGET_ONOFF = 1;
        globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_AUTO_TRACK;
        target_engine->cfg->b_sot_on = RK_TRUE;
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    case E_FK_TK_SINGLE_CLOSE:  //关闭单目标跟踪
    {
        RK_LOGE("E_FK_TK_SINGLE_CLOSE");

        globalMsg->m_algoControlMsg.e_TARGET_ONOFF = 2;
        target_engine->cfg->b_sot_on = RK_FALSE;
        globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_MANUAL;
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    case E_FK_TK_POSITION_OPEN:  //开启目标定位
    {
        RK_LOGE("E_FK_TK_POSITION_OPEN");

        globalMsg->m_algoControlMsg.e_LOC_ONOFF = 1;
        target_engine->cfg->b_location_on = RK_TRUE;
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    case E_FK_TK_POSITION_CLOSE:  //关闭目标定位
    {
        RK_LOGE("E_FK_TK_POSITION_CLOSE");

        globalMsg->m_algoControlMsg.e_LOC_ONOFF = 2;
        target_engine->cfg->b_location_on = RK_FALSE;
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    case E_FK_GEO_FOLLOW_OPEN:  //开启地理跟踪
    {
        RK_LOGE("E_FK_GEO_FOLLOW_OPEN");

        globalMsg->m_algoControlMsg.e_Geo_ONOFF = 1;
        geo_track->cfg->b_geotrack_on = RK_TRUE;
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    case E_FK_GEO_FOLLOW_CLOSE:  //关闭地理跟踪
    {
        RK_LOGE("E_FK_GEO_FOLLOW_CLOSE");
        globalMsg->m_algoControlMsg.e_Geo_ONOFF = 2;
        geo_track->cfg->b_geotrack_on = RK_FALSE;
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    default:
        break;
    }
}

void XKMsg::processMainView(XKDownMsg* _xkDownMsg)
{
    switch (_xkDownMsg->msg_type)
    {
    case E_FK_IR_SWITCH:  //切换到红外主视图
    {
        RK_LOGE("E_FK_IR_SWITCH OPEN");
        globalMsg->m_picCtlMsg.e_Switch_MainScreen = 2;   //全屏显示红外图像
        globalMsg->m_mainViewState.MainView = 2;             //主视图通道状态
        g_video_osd->cfg->setMainView(2);                          //OSD主视图切换
        target_engine->cfg->setMainView(2);
        ai_engine->cfg->setMainView(2);
        //同步状态到BC板
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    case E_FK_VI_SWITCH:  //切换到可见光主视图
    {
        RK_LOGE("E_FK_VI_SWITCH OPEN");
        globalMsg->m_picCtlMsg.e_Switch_MainScreen = 1;
        globalMsg->m_mainViewState.MainView = 1;
        g_video_osd->cfg->setMainView(1);
        target_engine->cfg->setMainView(1);
        ai_engine->cfg->setMainView(1);
        //同步状态到BC板
        syncGlobalToBoardLan();
        boardabcMsg->Send_A_to_B((void*)boardabcMsg->boardA_to_BC_Msg, sizeof(BOARDA_TO_BC_MSG));
        break;
    }
    default:
        break;
    }
}

void XKMsg::processPower(XKDownMsg* _xkDownMsg)
{
    bool sf_on = false;
    bool payload = true;
    bool sar_on = false;
    switch (_xkDownMsg->msg_type)
    {
    case E_FK_VL_OPEN:  //开启可见光
    {
        globalMsg->m_senerPowerState.e_CCD_on = 1;

        break;
    }
    case E_FK_VL_CLOSE:  //关闭可见光
    {
        globalMsg->m_senerPowerState.e_CCD_on = 2;

        break;
    }
    case E_FK_IR_OPEN:  //开启红外
    {
        globalMsg->m_senerPowerState.e_IR_on = 1;

        break;
    }
    case E_FK_IR_CLOSE:  //关闭红外
    {
        globalMsg->m_senerPowerState.e_IR_on = 2;

        break;
    }
    case E_FK_SF_OPEN:  //开启伺服
    {
        globalMsg->m_senerPowerState.e_SF_on = 1;
        //power_msg->ControlPower(payload, sf_on, sar_on, sar_on);
        break;
    }
    case E_FK_SF_CLOSE:  //关闭伺服
    {
        globalMsg->m_senerPowerState.e_SF_on = 2;
        //power_msg->ControlPower(payload, sf_on, sar_on, sar_on);
        break;
    }
    case E_FK_LASER_ON:  //开启激光测距
    {
        //printf("laser power on !\n");
        globalMsg->m_senerPowerState.e_Laser_on = 1;
        break;
    }
    case E_FK_LASER_OFF:  //关闭激光测距
    {
        //printf("laser power off !\n");
        globalMsg->m_senerPowerState.e_Laser_on = 2;
        break;
    }

    default:
        break;
    }
}

void XKMsg::recvXkThread()
{
    uint8_t data[4096] = { 0 };
    while (1)
    {
        int ret = iSocket::recv_some(m_socket_xk, data, 4096);
        if (ret > 0)
        {
            if (ret == sizeof(XKDownMsg))
            {
                memcpy(m_xkDownMsg, data, ret);
                //RK_LOGE("iSocket::recv_some msg_type : 0x%02X", m_xkDownMsg->msg_type);
                // 处理可见光
                processCCD(m_xkDownMsg);
                // 处理红外
                processIR(m_xkDownMsg);
                // 处理激光测距
                processJGCZ(m_xkDownMsg);               
                // 处理伺服
                processSF(m_xkDownMsg);
                // 处理电源控制
                processPower(m_xkDownMsg);

                // 处理主视图切换
                processMainView(m_xkDownMsg);
                
                // 处理算法控制
                processAlgo(m_xkDownMsg);

            }
            usleep(10000);
        }
    }
}

void XKMsg::globalRealTimeStatesUp() {
    memset(m_picupRealTimeStateMsgStruct, 0, sizeof(PICUpRealTimeStateMsgStruct));
    m_picupRealTimeStateMsgStruct->sync_code = 0x90;
    m_picupRealTimeStateMsgStruct->type = FKUP_REALTIME_STATE_E;

    while (1)
    {
        //传感器电源状态
        m_picupRealTimeStateMsgStruct->e_Laser_on = globalMsg->m_senerPowerState.e_Laser_on;
        m_picupRealTimeStateMsgStruct->e_CCD_on = globalMsg->m_senerPowerState.e_CCD_on;
        m_picupRealTimeStateMsgStruct->e_IR_on = globalMsg->m_senerPowerState.e_IR_on;
        m_picupRealTimeStateMsgStruct->e_SF_on = globalMsg->m_senerPowerState.e_SF_on;


        // 伺服
        sf_msg->recv_mtx.lock();
        m_picupRealTimeStateMsgStruct->f_PitchRate_value = globalMsg->m_sfMsg.f_PitchRate_value;
        m_picupRealTimeStateMsgStruct->f_Pitch_value = globalMsg->m_sfMsg.f_Pitch_value;
        m_picupRealTimeStateMsgStruct->f_AziRate_value = globalMsg->m_sfMsg.f_AziRate_value;
        m_picupRealTimeStateMsgStruct->f_Azi_value = globalMsg->m_sfMsg.f_Azi_value;
        m_picupRealTimeStateMsgStruct->state = sf_msg->statusMsg->STATUS;
        m_picupRealTimeStateMsgStruct->mode = sf_msg->statusMsg->WORK_MODE;
        sf_msg->recv_mtx.unlock();

        //可见光
        ccdmsg->recv_mtx.lock();
        m_picupRealTimeStateMsgStruct->f_VIView_Azi = globalMsg->m_camVIMsg.f_VIView_Azi;
        m_picupRealTimeStateMsgStruct->f_VIView_Pitch = globalMsg->m_camVIMsg.f_VIView_Pitch;
        m_picupRealTimeStateMsgStruct->f_VIZoomValue = globalMsg->m_camVIMsg.f_VIZoomValue;
        ccdmsg->recv_mtx.unlock();

        //红外
        irmsg->recv_mtx.lock();
        m_picupRealTimeStateMsgStruct->f_IRZView_Azi = globalMsg->m_camIRMsg.f_IRZView_Azi;
        m_picupRealTimeStateMsgStruct->f_IRZView_Pitch = globalMsg->m_camIRMsg.f_IRZView_Pitch;
        m_picupRealTimeStateMsgStruct->f_IRZoomValue = globalMsg->m_camIRMsg.f_IRZoomValue;
        irmsg->recv_mtx.unlock();

        //激光测距
        m_picupRealTimeStateMsgStruct->laser_distance = globalMsg->m_laserStateMsg.f_Dis_value;
        m_picupRealTimeStateMsgStruct->laser_isloopDis = globalMsg->m_laserStateMsg.b_isloopDis;
        m_picupRealTimeStateMsgStruct->laser_workModeState = globalMsg->m_laserStateMsg.uc_workModeState;
        m_picupRealTimeStateMsgStruct->laser_powerState = globalMsg->m_laserStateMsg.uc_powerState;
        m_picupRealTimeStateMsgStruct->laser_prepareState = globalMsg->m_laserStateMsg.uc_prepareState;
        m_picupRealTimeStateMsgStruct->laser_lockStatus = globalMsg->m_laserStateMsg.uc_lockStatus;
        if (globalMsg->m_laserStateMsg.uc_powerState == 0
            || globalMsg->m_laserStateMsg.uc_lockStatus==1)
        {
            m_picupRealTimeStateMsgStruct->laser_distance = 0;
            m_picupRealTimeStateMsgStruct->laser_isloopDis = false;
        }

        //算法状态
        m_picupRealTimeStateMsgStruct->e_AI_ONOFF = globalMsg->m_algoControlMsg.e_AI_ONOFF;
        m_picupRealTimeStateMsgStruct->e_TARGET_ONOFF = globalMsg->m_algoControlMsg.e_TARGET_ONOFF;
        m_picupRealTimeStateMsgStruct->e_LOC_ONOFF = globalMsg->m_algoControlMsg.e_LOC_ONOFF;
        m_picupRealTimeStateMsgStruct->e_Geo_ONOFF = globalMsg->m_algoControlMsg.e_Geo_ONOFF;
        
        const void* data = (void*)m_picupRealTimeStateMsgStruct;
        iSocket::sendto_some(m_socket_xk, data, sizeof(PICUpRealTimeStateMsgStruct),
            g_configManager->m_gd_configinfo->GD_Send_MNXK_Port,
            g_configManager->m_gd_configinfo->MNXK_IP);

        usleep(50000);
    }

}

void XKMsg::globalSelfCheckStatesUp()
{
    //memset(m_picupSelfCheckStateStruct, 0, sizeof(PICUpSelfCheckStateStruct));
    m_picupSelfCheckStateStruct->sync_code = 0x90;
    m_picupSelfCheckStateStruct->type = FKUP_SELFCHECK_STATE_E;

    m_picupSelfCheckStateStruct->e_SocA_state = globalMsg->m_cycleCheckMsg.e_SocA_state;
    m_picupSelfCheckStateStruct->e_SocB_state = globalMsg->m_cycleCheckMsg.e_SocB_state;

    if (globalMsg->m_senerPowerState.e_Laser_on == 2)
    {
        m_picupSelfCheckStateStruct->e_Laser_state = 1;
    } 
    else
    {
        m_picupSelfCheckStateStruct->e_Laser_state = globalMsg->m_cycleCheckMsg.e_Laser_state;
    }
    if (globalMsg->m_senerPowerState.e_CCD_on == 2)
    {
        m_picupSelfCheckStateStruct->e_CCD_state = 1;
    }
    else
    {
        m_picupSelfCheckStateStruct->e_CCD_state = globalMsg->m_cycleCheckMsg.e_CCD_state;
    }
    if (globalMsg->m_senerPowerState.e_IR_on == 2)
    {
        m_picupSelfCheckStateStruct->e_IR_state = 1;
    }
    else
    {
        m_picupSelfCheckStateStruct->e_IR_state = globalMsg->m_cycleCheckMsg.e_IR_state;
    }
    if (globalMsg->m_senerPowerState.e_SF_on == 2)
    {
        m_picupSelfCheckStateStruct->e_SF_state = 1;
    }
    else
    {
        m_picupSelfCheckStateStruct->e_SF_state = globalMsg->m_cycleCheckMsg.e_SF_state;
    }
    m_picupSelfCheckStateStruct->temperature = globalMsg->m_cycleCheckMsg.temperature;
    m_picupSelfCheckStateStruct->proc_voltage = globalMsg->m_cycleCheckMsg.proc_voltage;

    const void* data = (void*)m_picupSelfCheckStateStruct;
    iSocket::sendto_some(m_socket_xk, data, sizeof(PICUpSelfCheckStateStruct),
        g_configManager->m_gd_configinfo->GD_Send_MNXK_Port,
        g_configManager->m_gd_configinfo->MNXK_IP);

}

void XKMsg::sendAck(uint8_t msg_type, XKErrorCode err)
{
    // 当前仅记录日志，后续可扩展返回状态
    RK_LOGE("XK应答类型:0x%02X 结果:%s 错误码:%d",
        msg_type, (err == XK_ERR_OK ? "成功" : "失败"), err);
}