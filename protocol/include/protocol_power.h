#ifndef PROTOCOL_POWER_H
#define PROTOCOL_POWER_H
#include <stdint.h>
#include "rk_debug.h"
#include  "serialport.h"
#include "uartextend.h"
// 命令ID
typedef enum POWER_CMD
{
    // 查询命令(0xA1模式)
    E_POWER_QUERY = 0x00,          // 查询模式默认命令

    // 控制命令(0xA2模式)
    E_POWER_CONTROL = 0xFF,        // 控制模式默认命令

    // 控制数值定义
    E_POWER_ON = 0x99,            // 上电控制值
    E_POWER_OFF = 0x00            // 下电控制值
} POWER_CMD;

#pragma pack(push)
#pragma pack(1)

// 基础帧头
typedef struct POWER_FrameHeader
{
    uint8_t header1;         // 第一个帧头字节(0xD7)
    uint8_t header2;         // 第二个帧头字节(0xD7)
    uint8_t length;          // 帧长度
    uint8_t mode;           // 模式码(0xA1/0xA2)
    uint8_t command;        // 命令码
} POWER_FrameHeader;

// 电压电流数据结构
typedef struct POWER_Data
{
    uint8_t decimal;        // 小数部分
    uint8_t integer;        // 整数部分(温度最高位为符号位,1表示负)
} POWER_Data;

// 查询命令帧结构
typedef struct POWER_QueryMsg
{
    POWER_FrameHeader header;   // 帧头
    uint8_t checksum;          // 校验和
} POWER_QueryMsg;

// 控制命令帧结构
typedef struct POWER_ControlMsg
{
    POWER_FrameHeader header;   // 帧头
    uint8_t payload_power;      // 载荷供电48V控制
    uint8_t servo_power;        // 伺服稳定平台28V控制
    uint8_t sar_proc_power;     // SAR雷达处理28V控制
    uint8_t sar_rf_power;       // SAR雷达射频28V控制
    uint8_t checksum;          // 校验和
} POWER_ControlMsg;

// 响应帧结构
typedef struct POWER_Response
{
    POWER_FrameHeader header;      // 帧头
    POWER_Data payload_voltage;    // 载荷供电电压
    uint8_t reserved[2];           // 预留数据(默认0)
    POWER_Data servo_voltage;      // 伺服稳定平台电压
    POWER_Data servo_current;      // 伺服稳定平台电流
    POWER_Data proc_voltage;       // 综合处理模块电压
    POWER_Data proc_current;       // 综合处理模块电流
    POWER_Data fanA_voltage;       // 电子舱风扇A电压
    POWER_Data fanB_voltage;       // 电子舱风扇B电压
    POWER_Data sar_proc_voltage;   // SAR雷达处理电压
    POWER_Data sar_proc_current;   // SAR雷达处理电流
    POWER_Data sar_rf_voltage;     // SAR雷达射频电压
    POWER_Data sar_rf_current;     // SAR雷达射频电流
    POWER_Data temperature;        // 模块温度
    uint8_t checksum;              // 校验和
} POWER_Response;

#pragma pack(pop)

class POWER_msg {
public:
    POWER_msg();
    ~POWER_msg();

     //消息结构体
    POWER_QueryMsg* queryMsg;      // 查询消息
    POWER_ControlMsg* controlMsg;  // 控制消息
    POWER_Response* response;      // 响应消息

    void InitQueryMsg();           // 初始化查询消息
    void InitControlMsg();         // 初始化控制消息

    void SendMsg(uint8_t* msg, size_t length);   // 发送消息
    void RecvMsg();                              // 接收消息
    void queryPowerThread();
    void updatePowerState();
    uint8_t CalcChecksum(uint8_t* data, uint8_t len);  // 计算校验和

    // 查询命令接口
    void QueryStatus();            // 查询状态

    // 控制命令接口
    void ControlPower(bool payload_on, bool servo_on, bool sar_proc_on, bool sar_rf_on); // 控制电源

    // 数据解析
    float ParseData(const POWER_Data* data, bool isTemperature = false);  // 解析数据
    void ParseResponse(uint8_t* data);           // 解析响应数据

private:
    void ParseResponseData();      // 解析响应中的数据

private:
    //串口通信
    int power_fd;

};

#endif // PROTOCOL_POWER_H