#include "protocol_vi.h"
#include <cstring>
#include <unistd.h>

extern GlobalReatimeMsg* globalMsg;

Vi_msg::Vi_msg() {
    msgTx = new MSG_TX();
    msgRx = new MSG_RX();
    
    // 初始化状态变量
    m_zooming = false;
    m_focusing = false;
    m_filter_moving = false;
    m_ezoom_on = false;
    m_self_checking = false;
    m_manual_exp = false;
    m_preset_exec = false;
    m_cross_on = false;
    
    m_filter_state = 0;
    m_edefog_state = 0;
    m_enhance_state = 0;
    m_ezoom_state = 0;
    
    m_zoom_fault = false;
    m_focus_fault = false;
    m_camera_fault = false;
    m_control_fault = false;
    m_filter_fault = 0;
    m_check_ok = false;
    
    m_focal_length = 0.0f;
    m_focus_position = 0;
    m_gain_value = 0;
    m_exposure_value = 1;
    m_brightness_value = 128;

    m_zoom_position = 0; 
    m_curveConfigPath = "config/focus_curve.ini"; // 默认路径 config/focus_curve.ini
}

Vi_msg::~Vi_msg() {
    delete msgTx;
    delete msgRx;
}

void Vi_msg::initMsg() {
    memset(msgTx, 0, sizeof(MSG_TX));
    msgTx->HEAD1 = HEAD1_VALUE;
    msgTx->HEAD2 = HEAD2_TX_VALUE;
    msgTx->RESERVED = 0x00;
}

uint8_t Vi_msg::calculateChecksum(const uint8_t* data, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

void Vi_msg::sendMsg(const uint8_t* msg, size_t length) {
    int ret = send_to_dev(UART_INDEX_CCD, (char*)msg, length, 0);
    if (ret < 0) {
        RK_LOGE("VI SERIAL SEND ERROR! ERROR CODE:%d", ret);
    }
    usleep(20 * 1000); // 协议要求两条指令周期间隔≥20ms
}

void Vi_msg::initSetting() {
    // 初始化默认设置
    setExposureAuto();          // 默认自动调光
    setElectronicZoom(0);       // 默认电子变倍关
    setImageEnhanceOff();       // 默认图像增强关
    setPictureFlipNormal();     // 默认图像方向正常
}

uint16_t Vi_msg::makeShort(uint8_t high, uint8_t low) {
    return (uint16_t)((high << 8) | low);
}

void Vi_msg::splitShort(uint16_t value, uint8_t& high, uint8_t& low) {
    high = (value >> 8) & 0xFF;
    low = value & 0xFF;
}

// ========== 变焦控制 ==========
void Vi_msg::zoomStop() {
    initMsg();
    msgTx->CMD = CMD_IDLE;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::zoomTele() {
    initMsg();
    msgTx->CMD = CMD_ZOOM_PLUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::zoomWide() {
    initMsg();
    msgTx->CMD = CMD_ZOOM_MINUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::zoomDirect(float focal_length) {
    initMsg();
    msgTx->CMD = CMD_ZOOM_POS;
    
    // 焦距值为实际值的10倍
    uint16_t focal_value = (uint16_t)(focal_length * 10.0f);
    msgTx->DATA_LOW = focal_value & 0xFF;
    msgTx->DATA_HIGH = (focal_value >> 8) & 0xFF;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 电子变倍控制 ==========
void Vi_msg::setElectronicZoom(uint8_t mode) {
    initMsg();
    
    switch(mode) {
        case 0:
            msgTx->CMD = CMD_EZOOM_OFF;
            break;
        case 1:
            msgTx->CMD = CMD_EZOOM_2X;
            break;
        case 2:
            msgTx->CMD = CMD_EZOOM_4X;
            break;
        default:
            msgTx->CMD = CMD_EZOOM_OFF;
            break;
    }
    
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 聚焦控制 ==========
void Vi_msg::focusStop() {
    initMsg();
    msgTx->CMD = CMD_IDLE;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::focusFar() {
    initMsg();
    msgTx->CMD = CMD_FOCUS_PLUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::focusNear() {
    initMsg();
    msgTx->CMD = CMD_FOCUS_MINUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::focusDirect(uint16_t position) {
    initMsg();
    msgTx->CMD = CMD_FOCUS_POS;
    msgTx->DATA_LOW = position & 0xFF;
    msgTx->DATA_HIGH = (position >> 8) & 0xFF;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::focusOnePush() {
    initMsg();
    msgTx->CMD = CMD_FOCUS_AUTO;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 预置位控制 ==========
void Vi_msg::setPresetPosition(uint8_t id) {
    if (id < 1 || id > 20) {
        RK_LOGE("Invalid preset ID: %d (valid range: 1-20)", id);
        return;
    }
    
    initMsg();
    msgTx->CMD = CMD_PRESET_SET;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = id;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::callPresetPosition(uint8_t id) {
    if (id < 1 || id > 20) {
        RK_LOGE("Invalid preset ID: %d (valid range: 1-20)", id);
        return;
    }
    
    initMsg();
    msgTx->CMD = CMD_PRESET_CALL;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = id;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 曝光控制 ==========
void Vi_msg::setExposureAuto() {
    initMsg();
    msgTx->CMD = CMD_EXPOSURE_AUTO;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setExposureManual() {
    initMsg();
    msgTx->CMD = CMD_EXPOSURE_MANUAL;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setBrightnessPlus() {
    initMsg();
    msgTx->CMD = CMD_BRIGHTNESS_PLUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setBrightnessMinus() {
    initMsg();
    msgTx->CMD = CMD_BRIGHTNESS_MINUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setBrightnessDefault() {
    initMsg();
    msgTx->CMD = CMD_BRIGHTNESS_DEF;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setGainPlus() {
    initMsg();
    msgTx->CMD = CMD_GAIN_PLUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setGainMinus() {
    initMsg();
    msgTx->CMD = CMD_GAIN_MINUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setGainValue(uint8_t value) {
    if (value > 240) {
        RK_LOGE("Invalid gain value: %d (valid range: 0-240)", value);
        value = 240;
    }
    
    initMsg();
    msgTx->CMD = CMD_GAIN_SET;
    msgTx->DATA_LOW = value;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setShutterPlus() {
    initMsg();
    msgTx->CMD = CMD_SHUTTER_PLUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setShutterMinus() {
    initMsg();
    msgTx->CMD = CMD_SHUTTER_MINUS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setShutterValue(uint16_t value) {
    if (value < 1 || value > 20000) {
        RK_LOGE("Invalid shutter value: %d (valid range: 1-20000μs)", value);
        if (value < 1) value = 1;
        if (value > 20000) value = 20000;
    }
    
    initMsg();
    msgTx->CMD = CMD_SHUTTER_SET;
    msgTx->DATA_LOW = value & 0xFF;
    msgTx->DATA_HIGH = (value >> 8) & 0xFF;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 滤色片控制 ==========
void Vi_msg::setFilterVisible() {
    initMsg();
    msgTx->CMD = CMD_FILTER_VIS;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setFilterNearInfrared() {
    initMsg();
    msgTx->CMD = CMD_FILTER_NIR;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 透雾控制 ==========
void Vi_msg::setElectronicDefogOff() {
    initMsg();
    msgTx->CMD = CMD_EDEFOG_OFF;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setElectronicDefogOn() {
    initMsg();
    msgTx->CMD = CMD_EDEFOG_ON;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 图像增强控制 ==========
void Vi_msg::setImageEnhanceOff() {
    initMsg();
    msgTx->CMD = CMD_ENHANCE_OFF;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setImageEnhanceWeak() {
    initMsg();
    msgTx->CMD = CMD_ENHANCE_WEAK;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setImageEnhanceMid() {
    initMsg();
    msgTx->CMD = CMD_ENHANCE_MID;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setImageEnhanceStrong() {
    initMsg();
    msgTx->CMD = CMD_ENHANCE_STRONG;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setImageEnhanceCycle() {
    initMsg();
    msgTx->CMD = CMD_ENHANCE_CYCLE;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 图像翻转控制 ==========
void Vi_msg::setPictureFlipNormal() {
    initMsg();
    msgTx->CMD = CMD_FLIP_NORMAL;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setPictureFlipH() {
    initMsg();
    msgTx->CMD = CMD_FLIP_H;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setPictureFlipV() {
    initMsg();
    msgTx->CMD = CMD_FLIP_V;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setPictureFlipMirror() {
    initMsg();
    msgTx->CMD = CMD_FLIP_MIRROR;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 十字丝控制 ==========
void Vi_msg::setCrossHairOff() {
    initMsg();
    msgTx->CMD = CMD_CROSS_OFF;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

void Vi_msg::setCrossHairOn() {
    initMsg();
    msgTx->CMD = CMD_CROSS_ON;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 自检控制 ==========
void Vi_msg::startSelfCheck() {
    initMsg();
    msgTx->CMD = CMD_SELF_CHECK;
    msgTx->DATA_LOW = 0x00;
    msgTx->DATA_HIGH = 0x00;
    msgTx->PRESET_ID = 0x00;
    msgTx->CHECKSUM = calculateChecksum((uint8_t*)msgTx, 7);
    sendMsg((uint8_t*)msgTx, sizeof(MSG_TX));
}

// ========== 状态解析和更新 ==========
void Vi_msg::parseRxMsg(uint8_t* buf, int len) {
    if (len < 15) {
        RK_LOGE("Invalid message length: %d (expected 15)", len);
        return;
    }
    
    // 查找帧头
    int frame_start = -1;
    for (int i = 0; i <= len - 15; i++) {
        if (buf[i] == HEAD1_VALUE && buf[i+1] == HEAD2_RX_VALUE && buf[i+2] == FRAME_LENGTH) {
            frame_start = i;
            break;
        }
    }
    
    if (frame_start == -1) {
        RK_LOGE("Frame header not found");
        return;
    }
    
    // 验证校验和
    uint8_t checksum = calculateChecksum(&buf[frame_start], 14);
    if (checksum != buf[frame_start + 14]) {
        RK_LOGE("Checksum error: calculated=0x%02X, received=0x%02X", checksum, buf[frame_start + 14]);
        return;
    }
    
    recv_mtx.lock();
    memcpy(msgRx, &buf[frame_start], sizeof(MSG_RX));
    
    // 解析功能状态反馈1
    STATUS1_BITS* status1 = (STATUS1_BITS*)&msgRx->STATUS1;
    m_zooming = status1->zooming;
    m_focusing = status1->focusing;
    m_filter_moving = status1->filter_moving;
    m_ezoom_on = status1->ezoom_on;
    m_self_checking = status1->self_checking;
    m_manual_exp = status1->manual_exp;
    m_preset_exec = status1->preset_exec;
    m_cross_on = status1->cross_on;
    
    // 解析功能状态反馈2
    STATUS2_BITS* status2 = (STATUS2_BITS*)&msgRx->STATUS2;
    m_filter_state = status2->filter_state;
    m_edefog_state = status2->edefog_state;
    m_enhance_state = status2->enhance_state;
    m_ezoom_state = status2->ezoom_state;
    
    // 解析自检状态
    SELFCHECK_BITS* selfcheck = (SELFCHECK_BITS*)&msgRx->SELF_CHECK;
    m_zoom_fault = selfcheck->zoom_fault;
    m_focus_fault = selfcheck->focus_fault;
    m_camera_fault = selfcheck->camera_fault;
    m_control_fault = selfcheck->control_fault;
    m_filter_fault = selfcheck->filter_fault;
    m_check_ok = selfcheck->check_ok;
    
    uint16_t focal_raw = makeShort(msgRx->FOCAL_HIGH, msgRx->FOCAL_LOW); // 解析焦距值（低字节在前）
    m_focal_length = focal_raw / 10.0f;  // 实际值的10倍
    
    m_zoom_position = focal_raw; // 将原始焦距值作为 Zoom Position (用于查表)
    
    m_focus_position = makeShort(msgRx->FOCUS_HIGH, msgRx->FOCUS_LOW); // 解析聚焦位置（低字节在前）
    m_gain_value = msgRx->GAIN; // 解析增益值
    m_exposure_value = makeShort(msgRx->EXPOSURE_HIGH, msgRx->EXPOSURE_LOW); // 解析曝光值（低字节在前）
    m_brightness_value = msgRx->BRIGHTNESS; // 解析亮度值
    
    recv_mtx.unlock();
    #if 0
    printf("=== 功能状态反馈1 ===\n");
    printf("正在变焦：%s\n", status1->zooming ? "是" : "否");
    printf("正在聚焦：%s\n", status1->focusing ? "是" : "否");
    printf("正在调滤色片：%s\n", status1->filter_moving ? "是" : "否");
    printf("电子变倍：%s\n", status1->ezoom_on ? "开启" : "关闭");
    printf("正在自检：%s\n", status1->self_checking ? "是" : "否");
    printf("调光模式：%s\n", status1->manual_exp ? "手动调光" : "自动调光");
    printf("预置位调用：%s\n", status1->preset_exec ? "执行中" : "空闲中");
    printf("十字丝：%s\n", status1->cross_on ? "开启" : "关闭");
    printf("\n");
    printf("=== 功能状态反馈2 ===\n");
    switch (status2->filter_state) {
        case 0x00: printf("滤色片状态：可见光\n"); break;
        case 0x01: printf("滤色片状态：近红外\n");
        default:   printf("滤色片状态：未知（值：0x%02X）\n", status2->filter_state);
    }
    switch (status2->edefog_state) {
        case 0x00: printf("电子透雾：关闭\n"); break;
        case 0x01: printf("电子透雾：开启\n");
        default:   printf("电子透雾：未知（值：0x%02X）\n", status2->edefog_state);
    }
    switch (status2->enhance_state) {
        case 0x00: printf("增强模式：关\n"); break;
        case 0x01: printf("增强模式：弱\n"); break;
        case 0x02: printf("增强模式：中\n"); break;
        case 0x03: printf("增强模式：强\n"); break;
        default:   printf("增强模式：未知（值：0x%02X）\n", status2->enhance_state);
    }
    switch (status2->ezoom_state) {
        case 0x00: printf("电子变倍倍数：关\n"); break;
        case 0x01: printf("电子变倍倍数：2×\n"); break;
        case 0x02: printf("电子变倍倍数：4×\n"); break;
        default:   printf("电子变倍倍数：未知（值：0x%02X）\n", status2->ezoom_state);
    }
    printf("\n");
    printf("=== 自检状态 ===\n");
    printf("变焦故障：%s\n", selfcheck->zoom_fault ? "是" : "否");
    printf("聚焦故障：%s\n", selfcheck->focus_fault ? "是" : "否");
    printf("预留位（B2）：0x%01X（应恒为0）\n", selfcheck->reserved);  // 显式打印预留位
    printf("相机故障：%s\n", selfcheck->camera_fault ? "是" : "否");
    printf("控制模块故障：%s\n", selfcheck->control_fault ? "是" : "否");
    switch (selfcheck->filter_fault) {
        case 0x00: printf("滤色片状态：正常\n"); break;
        case 0x03: printf("滤色片状态：故障\n"); break;
        default:   printf("滤色片状态：未知（值：0x%02X）\n", selfcheck->filter_fault);
    }
    printf("自检结果：%s\n", selfcheck->check_ok ? "正常" : "不正常");
    printf("=================\n\n");
    #endif
    // RK_LOGD("Focal: %.1f, Focus: %d, Gain: %d, Exposure: %d, Brightness: %d",
    //         m_focal_length, m_focus_position, m_gain_value, m_exposure_value, m_brightness_value);
}

void Vi_msg::updateCommonState() {
    // 此处将当前状态更新到全局消息对象
    recv_mtx.lock();
    
    // globalMsg->m_camVIMsg.f_VIFocalLength = m_focal_length;
    // globalMsg->m_camVIMsg.f_VIFocusPosition = m_focus_position;
    
    recv_mtx.unlock();
}

void Vi_msg::recvCCDThread() {
    uint8_t buf[256] = {0};
    uint64_t last_recv_time = 0;
    uint64_t current_time;
    const int64_t MAX_TIME_DIFF = 5000;  // 5秒超时
    
    while (true) {
        memset(buf, 0, 256);
        int totalLen = 0;
        int ret = recv_from_dev(UART_INDEX_CCD, (char*)buf, &totalLen, 0);
        
        if (ret <= 0) {
            if (ret < 0) {
                RK_LOGE("VI SERIAL RECEIVE ERROR! code: %d", ret);
            }
            usleep(10000);
            continue;
        }
        
        // 更新接收时间，用于超时检测
        current_time = getCurTickCount();
        if (last_recv_time != 0) {
            int64_t time_diff = current_time - last_recv_time;
            if (time_diff > MAX_TIME_DIFF) {
                RK_LOGW("Communication timeout: %lld ms", time_diff);
                // globalMsg->m_cycleCheckMsg.e_CCD_state = 1;  // 设备异常
            } else {
                // globalMsg->m_cycleCheckMsg.e_CCD_state = 0;  // 设备正常
            }
        }
        last_recv_time = current_time;
        
        // 解析接收到的数据
        parseRxMsg(buf, totalLen);
        updateCommonState();
        
        usleep(10000);  // 10ms
    }
}

void Vi_msg::queryStateThread()
{
	while (true) {
		if (globalMsg->m_senerPowerState.e_CCD_on == 1)
		{
            
		}
		usleep(50000); // 主循环间隔
	}
}


void Vi_msg::LoadFocusCurve(const char* configPath) {
    m_curveConfigPath = configPath;
    std::ifstream infile(m_curveConfigPath);
    if (!infile.is_open()) {
        RK_LOGW("Vi_msg: Config %s not found, using empty curve.", configPath);
        return;
    }

    std::lock_guard<std::mutex> lock(m_curveMtx);
    m_focusCurve.clear();
    std::string line;
    int zoom, focus;
    
    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        if (iss >> zoom >> focus) {
            m_focusCurve[zoom] = focus;
        }
    }
    RK_LOGI("Vi_msg: Loaded %d points from focus curve.", m_focusCurve.size());
}

void Vi_msg::SaveFocusCurve() {
    std::lock_guard<std::mutex> lock(m_curveMtx);
    std::ofstream outfile(m_curveConfigPath);
    if (!outfile.is_open()) {
        RK_LOGE("Vi_msg: Failed to write config %s", m_curveConfigPath.c_str());
        return;
    }

    outfile << "# Zoom_Pos Focus_Pos\n";
    for (const auto& pair : m_focusCurve) {
        outfile << pair.first << " " << pair.second << "\n";
    }
    outfile.close();
    RK_LOGI("Vi_msg: Focus curve saved.");
}

uint16_t Vi_msg::GetBestFocus(uint16_t zoom_pos) {
    std::lock_guard<std::mutex> lock(m_curveMtx);
    if (m_focusCurve.empty()) return 0;

    // 查找第一个 key >= zoom_pos 的迭代器
    auto it = m_focusCurve.lower_bound(zoom_pos);

    if (it == m_focusCurve.begin()) return it->second;
    if (it == m_focusCurve.end()) return m_focusCurve.rbegin()->second;

    // 线性插值
    auto it_up = it;
    auto it_low = std::prev(it);

    int z1 = it_low->first;
    int f1 = it_low->second;
    int z2 = it_up->first;
    int f2 = it_up->second;

    if (z2 == z1) return f1;

    double ratio = (double)(zoom_pos - z1) / (double)(z2 - z1);
    return (uint16_t)(f1 + (f2 - f1) * ratio);
}

void Vi_msg::RunAutoCalibration() {
    RK_LOGI("Vi_msg: Start Auto Calibration...");
    
    // 1. 清空旧数据
    {
        std::lock_guard<std::mutex> lock(m_curveMtx);
        m_focusCurve.clear();
    }

    // 2. 标定流程 (简化版：假设从当前位置向广角端移动，实际应先移至长焦端)
    // 注意：实际项目中 Zoom 的范围和步长需要根据相机协议文档确定
    // 这里假设通过 zoomWide() 步进，或者通过 zoomDirect() 设定具体值
    // 假设我们标定 10 个点
    
    // 模拟：先移动到最长焦 (需根据实际相机行程时间调整等待)
    zoomTele(); 
    usleep(5000 * 1000); // 移动5秒
    zoomStop();
    usleep(1000 * 1000);

    for (int i = 0; i < 10; i++) {
        RK_LOGI("Calibrating point %d...", i);

        // A. 触发自动聚焦
        focusOnePush();
        usleep(3000 * 1000); // 等待聚焦完成

        // B. 查询并记录当前状态
        // 发送查询指令，等待 recv 线程更新 m_zoom_position 和 m_focus_position
        // 注意：这里需要确保 recv 线程正在运行
        usleep(500 * 1000); 
        
        int z = 0, f = 0;
        {
            std::lock_guard<std::mutex> lock(recv_mtx);
            z = m_zoom_position; // 使用解析出的焦距值作为 Zoom Key
            f = m_focus_position;
        }

        {
            std::lock_guard<std::mutex> lock(m_curveMtx);
            m_focusCurve[z] = f;
        }
        RK_LOGI("Point %d: Z=%d, F=%d", i, z, f);

        // C. 移动到下一个变倍位置 (向广角端移动)
        zoomWide();
        usleep(800 * 1000); // 移动0.8秒
        zoomStop();
        usleep(1000 * 1000); // 等待电机稳定
    }

    // 3. 保存
    SaveFocusCurve();
    RK_LOGI("Vi_msg: Calibration Finished.");
}