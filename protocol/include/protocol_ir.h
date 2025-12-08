#ifndef PROTOCOL_IR_H
#define PROTOCOL_IR_H

#include <stdint.h>
#include <mutex>
#include "rk_debug.h"
#include "xuart_api.h"
#include "protocol_common.h"

#pragma pack(push)
#pragma pack(1)


#define IR_START_FLAG       0xF0    // 起始标识
#define IR_END_FLAG         0xFF    // 结束标识
#define IR_DEVICE_ADDR      0x26    // 设备地址
#define IR_ESCAPE_CHAR      0xF5    // 转义字符

// 命令ID定义
typedef enum IR_CMD
{
    // 基础控制命令
    E_IR_STATUS_QUERY = 0x00,           // 状态查询
    E_IR_FOCUS = 0x01,                  // 聚焦（调焦）
    E_IR_MANUAL_BG_CORRECT = 0x02,      // 背景校正
    E_IR_MANUAL_SHUTTER = 0x03,         // 手动校正
    E_IR_CROSSHAIR = 0x04,              // 十字线显示
    E_IR_POLARITY = 0x05,               // 极性设置（白热/黑热）
    E_IR_GAMMA = 0x06,                  // Gamma调节
    E_IR_AUTO_CORRECT = 0x07,           // 自动校正
    E_IR_EZOOM = 0x08,                  // 电子放大设置
    E_IR_CONTRAST = 0x09,               // 视频增益（对比度）
    E_IR_BRIGHTNESS = 0x0A,             // 视频亮度
    E_IR_CROSSHAIR_X = 0x0B,            // 十字叉横坐标
    E_IR_CROSSHAIR_Y = 0x0C,            // 十字叉纵坐标
    E_IR_ENHANCE = 0x0E,                // 图像增强
    E_IR_STOP = 0x10,                   // 聚焦变焦停止
    E_IR_ZOOM = 0x11,                   // 变焦
    E_IR_FOCUS_POSITION = 0x18,         // 调焦位置设置
    E_IR_ZOOM_POSITION = 0x18,          // 变倍位置设置（同一命令，首字节区分）
    E_IR_POSITION_QUERY = 0x1D,         // 查询调焦和变倍位置值
    E_IR_TRIGGER_AF = 0x34,             // 自动聚焦
    E_IR_AUTO_DEADPIXEL = 0x57,         // 自动盲元处理
    E_IR_ENHANCE_COEF = 0x77,           // DDE设置（图像增强系数）
    E_IR_SYSTEM_RESET = 0x80            // 系统复位
} IR_CMD;

// 控制消息
typedef struct IR_ControlMsg
{
    uint8_t head;           // 起始标识 0xF0
    uint8_t length;         // 数据长度（仅包含数据区长度，不含转义字符）
    uint8_t data[20];       // 数据区（包含设备地址、命令字和参数）
    uint8_t checksum;       // 校验和（data区累加和的低8位）
    uint8_t tail;           // 结束标识 0xFF
} IR_ControlMsg;

// 状态查询响应
typedef struct IR_StatusMsg
{
    uint8_t funcStatus;     // 功能状态字节
                            // B0: 十字叉显示(0关闭/1开启)
                            // B1: 极性(0白热/1黑热)
                            // B2: 自动校正(0关闭/1开启)
                            // B3: 电子放大(0关闭/1开启)
                            // B5: 图像增强(0关闭/1开启)
    uint8_t contrast;       // 视频增益（对比度）0-255
    uint8_t brightness;     // 视频亮度 0-255
    uint16_t crosshairX;    // 十字叉横坐标（先低后高）
    uint16_t crosshairY;    // 十字叉纵坐标（先低后高）
    uint8_t gamma;          // Gamma值 1-23，默认8
    uint8_t reserved[7];    // 保留字节
} IR_StatusMsg;

// 位置查询响应（命令0x1D，首字节0x06）
typedef struct IR_PositionMsg
{
    uint8_t cmdType;        // 命令类型 0x06
    int16_t coreTemp;       // 机芯温度
    uint16_t zoomPos;       // 变倍位置值
    uint16_t focusPos;      // 调焦位置值
} IR_PositionMsg;

// 位置范围查询响应（命令0x1D，首字节0x28）
typedef struct IR_PositionRangeMsg
{
    uint8_t cmdType;        // 命令类型 0x28
    uint16_t focusMin;      // 调焦位置最小值
    uint16_t focusMax;      // 调焦位置最大值
    uint16_t zoomMin;       // 变倍位置最小值
    uint16_t zoomMax;       // 变倍位置最大值
} IR_PositionRangeMsg;

#pragma pack(pop)


class IR_msg
{
public:
    IR_msg();
    ~IR_msg();

    // 基础消息处理
    void InitControlMsg();                                      // 初始化控制消息
    void SendMsg(uint8_t* msg, size_t length);                  // 发送消息
    void RecvMsg(const uint8_t* buf, int totalLen);                                             // 接收消息
    uint8_t CalcChecksum(const uint8_t* data, uint8_t len);    // 计算校验和
    void EscapeData(uint8_t* input, uint8_t inLen, 
                    uint8_t* output, uint8_t* outLen);          // 数据转义处理
    void UnescapeData(const uint8_t* input, uint8_t inLen, 
                      uint8_t* output, uint8_t* outLen);        // 数据反转义处理

    // 查询命令
    void QueryStatus();                                         // 状态查询（0x00）
    void QueryPosition();                                       // 查询调焦和变倍位置值（0x1D + 0x00）
    void QueryPositionRange();                                  // 查询位置值的最大最小值（0x1D + 0x20）

    // 聚焦变焦控制
    void SetFocus(bool farFocus);                              // 聚焦控制（0x01）true=调焦+/false=调焦-
    void SetZoom(bool zoomIn);                                 // 变焦控制（0x11）true=变焦+/false=变焦-
    void StopFocusZoom();                                      // 聚焦变焦停止（0x10）
    void SetFocusPosition(uint16_t position);                  // 调焦位置设置（0x18 + 0x12）
    void SetZoomPosition(uint16_t position);                   // 变倍位置设置（0x18 + 0x22）
    void TriggerAF();                                          // 触发自动聚焦（0x34）

    // 图像处理命令
    void ManualBGCorrect();                                    // 背景校正（0x02）
    void ManualShutter();                                      // 手动校正（0x03）
    void SetCrosshair(bool enable);                            // 十字线显示（0x04）
    void SetPolarity(bool blackHot);                           // 极性设置（0x05）0=白热/1=黑热
    void SetGamma(uint8_t value);                              // Gamma调节（0x06）1-23，默认8
    void SetAutoCorrect(bool enable);                          // 自动校正（0x07）
    void SetEZoom(uint8_t mode);                               // 电子放大（0x08）0=关/0xF=2倍/3=4倍
    void SetContrast(uint8_t value);                           // 视频增益（0x09）0-255，默认100
    void SetBrightness(uint8_t value);                         // 视频亮度（0x0A）0-255，默认130
    void SetCrosshairX(uint16_t x);                            // 十字叉横坐标（0x0B）0-65535
    void SetCrosshairY(uint16_t y);                            // 十字叉纵坐标（0x0C）0-65535
    void SetImageEnhance(bool enable);                         // 图像增强（0x0E）
    void SetEnhanceCoef(uint8_t coef);                         // DDE设置（0x77）0-255

    // 高级功能
    void AutoDeadPixel();                                      // 自动盲元处理（0x57）
    void SystemReset();                                        // 系统复位（0x80）

    // 获取状态信息
    const IR_StatusMsg& GetStatusMsg() const { return statusMsg; }
    const IR_PositionMsg& GetPositionMsg() const { return positionMsg; }
    const IR_PositionRangeMsg& GetPositionRangeMsg() const { return positionRangeMsg; }

    void queryIRStateThread();
    void recvIRMsgThread();                 // 红外相机接收实时状态线程
    void updateCommonState();               // 全局状态信息刷新

    std::mutex recv_mtx;                    // 接收互斥锁
    std::mutex send_mtx;                    // 发送互斥锁

private:
    // 解析响应消息
    void ParseStatusMsg(const uint8_t* data, uint8_t len);         // 解析状态消息
    void ParsePositionMsg(const uint8_t* data, uint8_t len);       // 解析位置消息
    void ParsePositionRangeMsg(const uint8_t* data, uint8_t len);  // 解析位置范围消息

    // 构建并发送命令
    void BuildAndSendCmd(uint8_t cmd);                              // 无参数命令
    void BuildAndSendCmd(uint8_t cmd, uint8_t param);               // 单字节参数命令
    void BuildAndSendCmd(uint8_t cmd, uint16_t param);              // 双字节参数命令
    void BuildAndSendCmd(uint8_t cmd, const uint8_t* params, 
                        uint8_t paramLen);                          // 多字节参数命令

    // 成员变量
    IR_ControlMsg controlMsg;               // 控制消息
    IR_StatusMsg statusMsg;                 // 状态消息
    IR_PositionMsg positionMsg;             // 位置消息
    IR_PositionRangeMsg positionRangeMsg;   // 位置范围消息
};

#endif // PROTOCOL_IR_H