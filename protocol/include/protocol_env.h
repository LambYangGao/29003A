#ifndef PROTOCOL_ENV_H
#define PROTOCOL_ENV_H
#include <stdint.h>
#include <stddef.h>
#include<cstddef>
#include "rk_debug.h"
#include "xuart_api.h"
// 命令ID
typedef enum ENV_CMD
{
    // 查询命令(0xA1模式)
    E_ENV_QUERY_MODEL = 0xF1,         // 查询产品型号
    E_ENV_QUERY_VERSION = 0xF2,       // 查询软件版本号
    E_ENV_QUERY_SERIAL = 0xF3,        // 查询序列号 
    E_ENV_QUERY_TEMP = 0xF4,          // 查询实时温度
    E_ENV_QUERY_PRESSURE = 0xF5,      // 查询实时气压
    E_ENV_QUERY_HUMIDITY = 0xF6,      // 查询实时湿度
    E_ENV_QUERY_VOLTAGE = 0xF7,       // 查询实时电压值
    E_ENV_QUERY_EXTREMUM = 0xF8,      // 查询温湿度、气压、电压极值数据

    // 周期上报命令(0xA2模式)
    E_ENV_PERIODIC_REPORT = 0xFF      // 周期上报命令
} ENV_CMD;

#pragma pack(push)
#pragma pack(1)

// 基础帧头(查询命令和普通响应)
typedef struct ENV_FrameHeader
{
    uint8_t header1;         // 第一个帧头字节(0xD9)
    uint8_t header2;         // 第二个帧头字节(0xD9) 
    uint8_t length;          // 帧长度
    uint8_t mode;            // 模式码(0xA1/0xA2)
    uint8_t command;         // 命令码
} ENV_FrameHeader;

// 周期上报帧头(特殊处理双字节长度)
typedef struct ENV_PeriodicHeader
{
    uint8_t header1;         // 第一个帧头字节(0xD9)
    uint8_t header2;         // 第二个帧头字节(0xD9) 
    uint8_t length_l;        // 帧长度低字节(0x20)
    uint8_t length_h;        // 帧长度高字节(0x00)
    uint8_t mode;            // 模式码(0xA2)
    uint8_t command;         // 命令码(0xFF)
} ENV_PeriodicHeader;

// 环境数据结构
typedef struct ENV_Data
{
    uint8_t decimal;         // 小数部分 
    uint8_t integer;         // 整数部分(温度最高位为符号位,1表示负)
} ENV_Data;

// 版本信息结构 
typedef struct ENV_VersionInfo
{
    uint16_t version;        // 2字节版本号
    uint32_t date;           // 4字节版本日期
} ENV_VersionInfo;

// 序列号信息结构
typedef struct ENV_SerialInfo
{
    uint16_t number;         // 2字节序列号
    uint32_t date;           // 4字节序列日期
} ENV_SerialInfo;

// 查询命令帧结构
typedef struct ENV_QueryMsg
{
    ENV_FrameHeader header;     // 帧头
    uint8_t checksum;           // 校验和
} ENV_QueryMsg;

// 环境参数查询响应结构(长度0x08)
typedef struct ENV_EnvResponse
{
    ENV_FrameHeader header;    // 帧头
    ENV_Data data;             // 环境数据
    uint8_t checksum;          // 校验和
} ENV_EnvResponse;

// 产品型号响应结构(长度0x12)
typedef struct ENV_ModelResponse
{
    ENV_FrameHeader header;    // 帧头
    uint8_t model[12];         // 12字节ASCII型号
    uint8_t checksum;          // 校验和
} ENV_ModelResponse;

// 版本信息响应结构(长度0x12)
typedef struct ENV_VersionResponse
{
    ENV_FrameHeader header;     // 帧头
    ENV_VersionInfo version;    // 版本信息
    uint8_t reserved[6];        // 6字节预留
    uint8_t checksum;           // 校验和
} ENV_VersionResponse;

// 序列号响应结构(长度0x12)
typedef struct ENV_SerialResponse
{
    ENV_FrameHeader header;     // 帧头
    ENV_SerialInfo serial;      // 序列号信息
    uint8_t reserved[6];        // 6字节预留
    uint8_t checksum;           // 校验和
} ENV_SerialResponse;

// 周期上报数据结构(长度0x20)
typedef struct ENV_PeriodicReport
{
    ENV_PeriodicHeader header;  // 周期上报帧头(带双字节长度)
    ENV_Data temperature;       // 温度数据
    ENV_Data pressure;          // 气压数据
    ENV_Data humidity;          // 湿度数据
    ENV_Data voltage;           // 电压数据
    ENV_VersionInfo version;    // 版本信息
    ENV_SerialInfo serial;      // 序列号信息
    uint8_t reserved[5];        // 5字节预留
    uint8_t checksum;           // 校验和
} ENV_PeriodicReport;

#pragma pack(pop)

class ENV_msg {
public:
    ENV_msg();
    ~ENV_msg();

    // 消息结构体 
    ENV_QueryMsg* queryMsg;               // 查询消息
    ENV_EnvResponse* envResponse;         // 环境参数响应
    ENV_ModelResponse* modelResponse;     // 产品型号响应
    ENV_VersionResponse* versionResponse; // 版本信息响应
    ENV_SerialResponse* serialResponse;   // 序列号响应
    ENV_PeriodicReport* periodicReport;   // 周期上报数据

    void InitQueryMsg();                  // 初始化查询消息

    void SendMsg(uint8_t* msg, size_t length);                     // 发送消息
    void RecvMsg();                                                         // 接收消息
    void hkRecvThread();                                                     //环控接收线程
    void hkQueryThread();                                                    //环控查询线程
    void updateCommonState();
    uint8_t CalcChecksum(uint8_t* data, uint8_t len);              // 计算校验和

    // 查询命令接口
    void QueryProductModel();                                      // 查询产品型号
    void QuerySoftwareVersion();                                   // 查询软件版本
    void QuerySerialNumber();                                      // 查询序列号
    void QueryTemperature();                                       // 查询实时温度
    void QueryPressure();                                          // 查询实时气压
    void QueryHumidity();                                          // 查询实时湿度
    void QueryVoltage();                                          // 查询实时电压
    void QueryExtremum();                                         // 查询极值数据

    // 周期上报命令
    void EnablePeriodicReport();                                  // 周期上报

    // 解析数据
    float ParseEnvData(const ENV_Data* data);                                   // 解析环境数据
    void ParseVersionInfo(const ENV_VersionInfo* info, char* buf, size_t size); // 解析版本信息
    void ParseSerialInfo(const ENV_SerialInfo* info, char* buf, size_t size);   // 解析序列号

private:
    void ParseEnvResponse(const uint8_t* data);                   // 解析环境参数响应
    void ParseModelResponse(const uint8_t* data);                 // 解析产品型号响应
    void ParseVersionResponse(const uint8_t* data);               // 解析版本信息响应
    void ParseSerialResponse(const uint8_t* data);                // 解析序列号响应
    void ParsePeriodicReport(const uint8_t* data);                // 解析周期上报数据
};

#endif // PROTOCOL_ENV_H