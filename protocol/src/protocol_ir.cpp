#include "protocol_ir.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "serialport.h"

extern GlobalReatimeMsg* globalMsg;

IR_msg::IR_msg() {
    memset(&controlMsg, 0, sizeof(IR_ControlMsg));
    memset(&statusMsg, 0, sizeof(IR_StatusMsg));
    memset(&positionMsg, 0, sizeof(IR_PositionMsg));
    memset(&positionRangeMsg, 0, sizeof(IR_PositionRangeMsg));
    InitControlMsg();
}

IR_msg::~IR_msg() {

}

// 初始化控制消息
void IR_msg::InitControlMsg() {
    controlMsg.head = IR_START_FLAG;
    controlMsg.tail = IR_END_FLAG;
    controlMsg.length = 0;
    memset(controlMsg.data, 0, sizeof(controlMsg.data));
    controlMsg.checksum = 0;
}

// 计算校验和（仅对数据区累加）
uint8_t IR_msg::CalcChecksum(const uint8_t* data, uint8_t len) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// 数据转义处理
void IR_msg::EscapeData(uint8_t* input, uint8_t inLen, uint8_t* output, uint8_t* outLen) {
    uint8_t idx = 0;
    for (uint8_t i = 0; i < inLen; i++) {
        if (input[i] == 0xF0) {
            output[idx++] = 0xF5;
            output[idx++] = 0x00;
        } else if (input[i] == 0xFF) {
            output[idx++] = 0xF5;
            output[idx++] = 0x0F;
        } else if (input[i] == 0xF5) {
            output[idx++] = 0xF5;
            output[idx++] = 0x05;
        } else {
            output[idx++] = input[i];
        }
    }
    *outLen = idx;
}

// 数据反转义处理
void IR_msg::UnescapeData(const uint8_t* input, uint8_t inLen, uint8_t* output, uint8_t* outLen) {
    uint8_t idx = 0;
    for (uint8_t i = 0; i < inLen; i++) {
        if (input[i] == 0xF5 && i + 1 < inLen) {
            if (input[i + 1] == 0x00) {
                output[idx++] = 0xF0;
                i++;
            } else if (input[i + 1] == 0x0F) {
                output[idx++] = 0xFF;
                i++;
            } else if (input[i + 1] == 0x05) {
                output[idx++] = 0xF5;
                i++;
            }
        } else {
            output[idx++] = input[i];
        }
    }
    *outLen = idx;
}

// 发送消息
void IR_msg::SendMsg(uint8_t* msg, size_t length) {
    std::lock_guard<std::mutex> lock(send_mtx); // 会自动调用send_mtx.lock()，超出作用域自动调用 send_mtx.unlock()
    
    uint8_t sendBuf[256];
    uint8_t escapedData[128];
    uint8_t escapedLen = 0;
    
    // 对数据区进行转义处理
    EscapeData(controlMsg.data, controlMsg.length, escapedData, &escapedLen);
    
    // 构建数据包
    uint8_t idx = 0;
    sendBuf[idx++] = controlMsg.head;          // 起始标志
    sendBuf[idx++] = controlMsg.length;        // 数据长度（未转义的长度）
    
    // 复制转义后的数据
    memcpy(&sendBuf[idx], escapedData, escapedLen);
    idx += escapedLen;
    
    sendBuf[idx++] = controlMsg.checksum;      // 校验和
    sendBuf[idx++] = controlMsg.tail;          // 结束标志
    
    // 调用实际的串口发送函数
    send_to_dev(UART_INDEX_IR, (char*)sendBuf, idx, 0);
    printf("IR Send: ");
    for (int i = 0; i < idx; i++) {
        printf("%02X ", sendBuf[i]);
    }
    printf("\n");
}

// 接收消息
void IR_msg::RecvMsg(const uint8_t* buf, int totalLen)
{
    if (totalLen < 6)
        return;

    // 起始标志
    if (buf[0] != IR_START_FLAG)
        return;

    uint8_t expectedDataLen = buf[1];
    if (expectedDataLen == 0 || expectedDataLen > 200)
        return;

    // 解析转义数据长度
    int pos = 2;
    int unescapedCount = 0;
    while (unescapedCount < expectedDataLen && pos < totalLen)
    {
        if (buf[pos] == 0xF5 && pos + 1 < totalLen)
            pos += 2;
        else
            pos++;

        unescapedCount++;
    }

    if (pos >= totalLen)
        return;

    int escapedDataLen = pos - 2;

    // 反转义
    uint8_t unescapedData[128];
    uint8_t unescapedLen = 0;
    UnescapeData(buf + 2, escapedDataLen, unescapedData, &unescapedLen);

    if (unescapedLen != expectedDataLen)
        return;

    // 校验和
    uint8_t checksum = CalcChecksum(unescapedData, expectedDataLen);
    if (checksum != buf[2 + escapedDataLen])
        return;

    // 结束标志
    if (buf[2 + escapedDataLen + 1] != IR_END_FLAG)
        return;

    // 设备地址
    if (unescapedData[0] != IR_DEVICE_ADDR)
        return;

    uint8_t cmd = unescapedData[1];

    // 调用解析函数
    switch (cmd)
    {
        case E_IR_STATUS_QUERY:
            ParseStatusMsg(unescapedData + 2, unescapedLen - 2);
            break;

        case E_IR_POSITION_QUERY:
        {
            uint8_t type = unescapedData[2];
            if (type == 0x06)
                ParsePositionMsg(unescapedData + 2, unescapedLen - 2);
            else if (type == 0x28)
                ParsePositionRangeMsg(unescapedData + 2, unescapedLen - 2);
            break;
        }

        default:
            break;
    }
}

// 构建并发送无参数命令
void IR_msg::BuildAndSendCmd(uint8_t cmd) {
    InitControlMsg();
    controlMsg.data[0] = IR_DEVICE_ADDR;
    controlMsg.data[1] = cmd;
    controlMsg.length = 2;
    controlMsg.checksum = CalcChecksum(controlMsg.data, controlMsg.length);
    SendMsg(controlMsg.data, controlMsg.length);
}

// 构建并发送单字节参数命令
void IR_msg::BuildAndSendCmd(uint8_t cmd, uint8_t param) {
    InitControlMsg();
    controlMsg.data[0] = IR_DEVICE_ADDR;
    controlMsg.data[1] = cmd;
    controlMsg.data[2] = param;
    controlMsg.length = 3;
    controlMsg.checksum = CalcChecksum(controlMsg.data, controlMsg.length);
    SendMsg(controlMsg.data, controlMsg.length);
}

// 构建并发送双字节参数命令（先低后高）
void IR_msg::BuildAndSendCmd(uint8_t cmd, uint16_t param) {
    InitControlMsg();
    controlMsg.data[0] = IR_DEVICE_ADDR;
    controlMsg.data[1] = cmd;
    controlMsg.data[2] = param & 0xFF;         // 低字节
    controlMsg.data[3] = (param >> 8) & 0xFF;  // 高字节
    controlMsg.length = 4;
    controlMsg.checksum = CalcChecksum(controlMsg.data, controlMsg.length);
    SendMsg(controlMsg.data, controlMsg.length);
}

// 构建并发送多字节参数命令
void IR_msg::BuildAndSendCmd(uint8_t cmd, const uint8_t* params, uint8_t paramLen) {
    InitControlMsg();
    controlMsg.data[0] = IR_DEVICE_ADDR;
    controlMsg.data[1] = cmd;
    memcpy(&controlMsg.data[2], params, paramLen);
    controlMsg.length = 2 + paramLen;
    controlMsg.checksum = CalcChecksum(controlMsg.data, controlMsg.length);
    SendMsg(controlMsg.data, controlMsg.length);
}

// ==================== 查询命令实现 ====================

// 状态查询（0x00）
void IR_msg::QueryStatus() {
    BuildAndSendCmd(E_IR_STATUS_QUERY);
}

// 查询调焦和变倍位置值（0x1D + 0x00）
void IR_msg::QueryPosition() {
    BuildAndSendCmd(E_IR_POSITION_QUERY, (uint8_t)0x00);
}

// 查询位置值的最大最小值（0x1D + 0x20）
void IR_msg::QueryPositionRange() {
    BuildAndSendCmd(E_IR_POSITION_QUERY, (uint8_t)0x20);
}

// ==================== 聚焦变焦控制实现 ====================

// 聚焦控制（0x01）
void IR_msg::SetFocus(bool farFocus) {
    uint8_t param = farFocus ? 0x00 : 0x0F;  // 0x00=调焦+, 0x0F=调焦-
    BuildAndSendCmd(E_IR_FOCUS, param);
}

// 变焦控制（0x11）
void IR_msg::SetZoom(bool zoomIn) {
    uint8_t param = zoomIn ? 0x00 : 0x0F;  // 0x00=变焦+, 0x0F=变焦-
    BuildAndSendCmd(E_IR_ZOOM, param);
}

// 聚焦变焦停止（0x10）
void IR_msg::StopFocusZoom() {
    BuildAndSendCmd(E_IR_STOP);
}

// 调焦位置设置（0x18 + 0x12 + 位置）
void IR_msg::SetFocusPosition(uint16_t position) {
    uint8_t params[3];
    params[0] = 0x12;                      // 调焦标识
    params[1] = position & 0xFF;           // 低字节
    params[2] = (position >> 8) & 0xFF;    // 高字节
    BuildAndSendCmd(E_IR_FOCUS_POSITION, params, 3);
}

// 变倍位置设置（0x18 + 0x22 + 位置）
void IR_msg::SetZoomPosition(uint16_t position) {
    uint8_t params[3];
    params[0] = 0x22;                      // 变倍标识
    params[1] = position & 0xFF;           // 低字节
    params[2] = (position >> 8) & 0xFF;    // 高字节
    BuildAndSendCmd(E_IR_ZOOM_POSITION, params, 3);
}

// 触发自动聚焦（0x34）
void IR_msg::TriggerAF() {
    BuildAndSendCmd(E_IR_TRIGGER_AF);
}

// ==================== 图像处理命令实现 ====================

// 背景校正（0x02）
void IR_msg::ManualBGCorrect() {
    BuildAndSendCmd(E_IR_MANUAL_BG_CORRECT);
}

// 手动校正（0x03）
void IR_msg::ManualShutter() {
    BuildAndSendCmd(E_IR_MANUAL_SHUTTER);
}

// 十字线显示（0x04）
void IR_msg::SetCrosshair(bool enable) {
    uint8_t param = enable ? 0x0F : 0x00;
    BuildAndSendCmd(E_IR_CROSSHAIR, param);
}

// 极性设置（0x05）
void IR_msg::SetPolarity(bool blackHot) {
    uint8_t param = blackHot ? 0x0F : 0x00;  // 0x00=白热, 0x0F=黑热
    BuildAndSendCmd(E_IR_POLARITY, param);
}

// Gamma调节（0x06）
void IR_msg::SetGamma(uint8_t value) {
    // 范围检查：1-23
    if (value < 1) value = 1;
    if (value > 23) value = 23;
    BuildAndSendCmd(E_IR_GAMMA, value);
}

// 自动校正（0x07）
void IR_msg::SetAutoCorrect(bool enable) {
    uint8_t param = enable ? 0x0F : 0x00;
    BuildAndSendCmd(E_IR_AUTO_CORRECT, param);
}

// 电子放大（0x08）
void IR_msg::SetEZoom(uint8_t mode) {
    // mode: 0x00=关, 0x0F=2倍, 0x03=4倍
    BuildAndSendCmd(E_IR_EZOOM, mode);
}

// 视频增益/对比度（0x09）
void IR_msg::SetContrast(uint8_t value) {
    // 范围：0-255，默认100
    BuildAndSendCmd(E_IR_CONTRAST, value);
}

// 视频亮度（0x0A）
void IR_msg::SetBrightness(uint8_t value) {
    // 范围：0-255，默认130
    BuildAndSendCmd(E_IR_BRIGHTNESS, value);
}

// 十字叉横坐标（0x0B）
void IR_msg::SetCrosshairX(uint16_t x) {
    BuildAndSendCmd(E_IR_CROSSHAIR_X, x);
}

// 十字叉纵坐标（0x0C）
void IR_msg::SetCrosshairY(uint16_t y) {
    BuildAndSendCmd(E_IR_CROSSHAIR_Y, y);
}

// 图像增强（0x0E）
void IR_msg::SetImageEnhance(bool enable) {
    uint8_t param = enable ? 0x0F : 0x00;
    BuildAndSendCmd(E_IR_ENHANCE, param);
}

// DDE设置/图像增强系数（0x77）
void IR_msg::SetEnhanceCoef(uint8_t coef) {
    // 范围：0-255
    BuildAndSendCmd(E_IR_ENHANCE_COEF, coef);
}

// ==================== 高级功能实现 ====================

// 自动盲元处理（0x57）
void IR_msg::AutoDeadPixel() {
    uint8_t params[3] = {0x64, 0x51, 0x62};
    BuildAndSendCmd(E_IR_AUTO_DEADPIXEL, params, 3);
}

// 系统复位（0x80）
void IR_msg::SystemReset() {
    BuildAndSendCmd(E_IR_SYSTEM_RESET);
}

// ==================== 解析响应消息 ====================

// 解析状态消息（0x00响应）
void IR_msg::ParseStatusMsg(const uint8_t* data, uint8_t len) {
    if (len < 16) {
        return;
    }
    
    statusMsg.funcStatus = data[0];
    statusMsg.contrast = data[1];
    statusMsg.brightness = data[2];
    statusMsg.crosshairX = data[3] | (data[4] << 8);  // 先低后高
    statusMsg.crosshairY = data[5] | (data[6] << 8);  // 先低后高
    statusMsg.gamma = data[7];
    memcpy(statusMsg.reserved, &data[8], 7);
    
    printf("IR Status: funcStatus=0x%02X, contrast=%d, brightness=%d, "
           "crosshairX=%d, crosshairY=%d, gamma=%d\n",
           statusMsg.funcStatus, statusMsg.contrast, statusMsg.brightness,
           statusMsg.crosshairX, statusMsg.crosshairY, statusMsg.gamma);
}

// 解析位置消息（0x1D响应，类型0x06）
void IR_msg::ParsePositionMsg(const uint8_t* data, uint8_t len) {
    if (len < 7) {
        return;
    }
    
    positionMsg.cmdType = data[0];  // 0x06
    positionMsg.coreTemp = data[1] | (data[2] << 8);      // 机芯温度
    positionMsg.zoomPos = data[3] | (data[4] << 8);       // 变倍位置
    positionMsg.focusPos = data[5] | (data[6] << 8);      // 调焦位置
    
    printf("IR Position: coreTemp=%d, zoomPos=%d, focusPos=%d\n",
           positionMsg.coreTemp, positionMsg.zoomPos, positionMsg.focusPos);
}

// 解析位置范围消息（0x1D响应，类型0x28）
void IR_msg::ParsePositionRangeMsg(const uint8_t* data, uint8_t len) {
    if (len < 9) {
        return;
    }
    
    positionRangeMsg.cmdType = data[0];  // 0x28
    positionRangeMsg.focusMin = data[1] | (data[2] << 8);
    positionRangeMsg.focusMax = data[3] | (data[4] << 8);
    positionRangeMsg.zoomMin = data[5] | (data[6] << 8);
    positionRangeMsg.zoomMax = data[7] | (data[8] << 8);
    
    printf("IR Position Range: focusMin=%d, focusMax=%d, zoomMin=%d, zoomMax=%d\n",
           positionRangeMsg.focusMin, positionRangeMsg.focusMax,
           positionRangeMsg.zoomMin, positionRangeMsg.zoomMax);
}

void IR_msg::queryIRStateThread()
{
    int counter = 0;

    while (true)
    {
        if (globalMsg->m_senerPowerState.e_IR_on == 1)
        {
            // 50ms 查询一次状态
            QueryStatus();
            usleep(50000);

            // 100ms 查询焦距/变倍位置
            if (counter % 2 == 0)
            {
                QueryPosition();
                usleep(50000);
            }

            // 500ms 查询最大最小值
            if (counter % 10 == 0)
            {
                QueryPositionRange();
                usleep(50000);
            }

            counter++;
            if (counter > 20000) counter = 0;
        }

        usleep(50000);
    }
}


void IR_msg::recvIRMsgThread()
{
    uint8_t buf[256] = {0};
    uint64_t lastRecvTime = 0;
    const int64_t MAX_TIME_DIFF = 5000;  // 5秒无数据 → 掉线

    while (true)
    {
        if (globalMsg->m_senerPowerState.e_IR_on == 1)
        {
            memset(buf, 0, sizeof(buf));
            int totalLen = 0;

            int ret = recv_from_dev(UART_INDEX_IR, (char*)buf, &totalLen, 0);
            if (ret <= 0)
            {
                usleep(20000);
                continue;
            }

            uint64_t cur = getCurTickCount();
            if (lastRecvTime != 0)
            {
                globalMsg->m_cycleCheckMsg.e_IR_state =
                    (cur - lastRecvTime > MAX_TIME_DIFF ? 1 : 0);
            }
            lastRecvTime = cur;


            RecvMsg(buf, totalLen); // 解析

            updateCommonState(); // 更新全局状态
        }

        usleep(20000);
    }
}

void IR_msg::updateCommonState()
{
    recv_mtx.lock();
    //globalMsg->m_camIRMsg.f_IRZoomValue = lensStatus->focalLen;
	//globalMsg->m_camIRMsg.f_IRZView_Azi = lensStatus->fieldAngle / 100.f;
	//globalMsg->m_camIRMsg.f_IRZView_Pitch = globalMsg->m_camIRMsg.f_IRZView_Azi * 512 / 640.f;
    //globalMsg->m_camIRMsg.IRImgMode = (statusMsg->funcStatus & 0x10) ? 1 : 0;
    recv_mtx.unlock();
	//RK_LOGE("f_IRZoomValue is :%dmm", globalMsg->m_camIRMsg.f_IRZoomValue);
	//RK_LOGE("f_IRZView_Azi is :%f", globalMsg->m_camIRMsg.f_IRZView_Azi);
	//RK_LOGE("f_IRZView_Pitch is :%f", globalMsg->m_camIRMsg.f_IRZView_Pitch);
}