#ifndef PROTOCOL_SF_H
#define PROTOCOL_SF_H
#include <stdint.h>
#include "rk_debug.h"
#include "uartextend.h"
#include <mutex>


#pragma pack(push)
#pragma pack(1)

// 命令枚举 
typedef enum SF_CMD
{
    E_SF_INVALID = 0x00,     // 无效指令
    E_SF_COLLECT = 0x01,     // 收藏
    E_SF_STOP = 0x02,        // 伺服停止
    E_SF_ZERO = 0x03,        // 归零 
    E_SF_TRACK = 0x04,       // 跟踪
    E_SF_FOLLOW = 0x05,      // 随动(锁角)
    E_SF_SCAN = 0x06,        // 方位扇形扫描
    E_SF_MEM_TRACK = 0x12,   // 记忆跟踪
    E_SF_VERSION = 0x18      // 查询版本号
} SF_CMD;

// 状态位定义
typedef enum SF_STATUS_BIT
{
    SF_STATUS_AZI_ENCODER = 0x01,    // D0: 方位编码器状态 (0:正常, 1:故障)
    SF_STATUS_PITCH_ENCODER = 0x02,   // D1: 俯仰编码器状态 (0:正常, 1:故障)
    SF_STATUS_AZI_DRIVER = 0x04,      // D2: 方位驱动器状态 (0:正常, 1:故障)
    SF_STATUS_PITCH_DRIVER = 0x08,    // D3: 俯仰驱动器状态 (0:正常, 1:故障)
    SF_STATUS_SERVO_BOARD = 0x10,     // D4: 伺服控制板状态 (0:正常, 1:故障)
    SF_STATUS_GYRO = 0x20,           // D5: 陀螺仪状态 (0:正常, 1:故障)
} SF_STATUS_BIT;

// 工作模式位定义
typedef enum SF_WORK_MODE
{
    SF_MODE_PITCH_SCAN = 0x01,      // D0: 1表示俯仰扇形扫描
    SF_MODE_COLLECT = 0x01,         // 收藏模式
    SF_MODE_STOP = 0x02,            // 伺服停止模式
    SF_MODE_ZERO = 0x03,            // 归零模式
    SF_MODE_TRACK = 0x04,           // 跟踪模式
    SF_MODE_FOLLOW = 0x05,          // 随动模式
    SF_MODE_AZI_SCAN = 0x06,        // 方位扇形扫描
    SF_MODE_MEM_TRACK = 0x12,       // 记忆跟踪模式
} SF_WORK_MODE;

// 状态上报消息结构
typedef struct SF_StatusMsg
{
    uint8_t HEAD;            // 帧头 0xA5
    uint8_t LENGTH;          // 帧长度 0x0D

    uint8_t STATUS;          // 状态上报 (参考 SF_STATUS_BIT)
    uint8_t WORK_MODE;       // 工作模式上报 (参考 SF_WORK_MODE)
    uint8_t AZI_HIGH;        // 编码器方位角高8位 (19位中的高3位)  
    uint8_t AZI_MID;         // 编码器方位角中8位
    uint8_t AZI_LOW;         // 编码器方位角低8位 
    uint8_t PITCH_HIGH;      // 编码器俯仰角高8位
    uint8_t PITCH_MID;       // 编码器俯仰角中8位 
    uint8_t PITCH_LOW;       // 编码器俯仰角低8位
    uint8_t AZI_GYRO_H;      // 陀螺仪方位角速度高8位 (-300°/s～+300°/s)
    uint8_t AZI_GYRO_L;      // 陀螺仪方位角速度低8位
    uint8_t PITCH_GYRO_H;    // 陀螺仪俯仰角速度高8位 (-300°/s～+300°/s)
    uint8_t PITCH_GYRO_L;    // 陀螺仪俯仰角速度低8位

    uint8_t CHECKSUM;        // 校验和 (LENGTH到最后一个数据字节的异或和)
    uint8_t TAIL;            // 帧尾 0x5A
} SF_StatusMsg;

// 版本查询响应消息结构
typedef struct SF_VersionMsg
{
    uint8_t HEAD;            // 帧头 0xA5
    uint8_t LENGTH;          // 帧长度 0x0D

    uint8_t SYNC1;           // 固定值 0x55
    uint8_t SYNC2;           // 固定值 0xAA
    uint8_t VERSION1;        // 版本号1
    uint8_t VERSION2;        // 版本号2  
    uint8_t VERSION3;        // 版本号3
    uint8_t VERSION4;        // 版本号4
    uint8_t YEAR_H;          // 年高位 (2024年为0x0E)
    uint8_t YEAR_L;          // 年低位 (2024年为0x7B)
    uint8_t MONTH;           // 月
    uint8_t DAY;             // 日
    uint8_t SYNC3;           // 固定值 0x55
    uint8_t SYNC4;           // 固定值 0xAA

    uint8_t CHECKSUM;        // 校验和
    uint8_t TAIL;            // 帧尾 0x5A
} SF_VersionMsg;

// 控制命令消息结构
typedef struct SF_ControlMsg
{
    uint8_t HEAD;            // 帧头 0xA5
    uint8_t LENGTH;          // 帧长 0x0A

    uint8_t CMD;             // 命令 (参考 SF_CMD)
    uint8_t PARAM1;          // 参数1
    uint8_t PARAM2;          // 参数2
    uint8_t PARAM3;          // 参数3
    uint8_t PARAM4;          // 参数4
    uint8_t PARAM5;          // 参数5
    uint8_t PARAM6;          // 参数6
    uint8_t PARAM7;          // 参数7
    uint8_t PARAM8;          // 参数8

    uint8_t CHECKSUM;        // 校验和 (LENGTH到PARAM8的异或和)
    uint8_t TAIL;            // 帧尾 0x5A
} SF_ControlMsg;

#pragma pack(pop)

// SF消息处理类
class SF_msg {
public:
    SF_msg();
    ~SF_msg();

    SF_StatusMsg* statusMsg;     // 状态信息
    SF_ControlMsg* controlMsg;   // 控制指令
    SF_VersionMsg* versionMsg;   // 版本信息

    void InitControlMsg();       // 初始化控制消息
    void CalcChecksum();         // 计算校验和

    // 具体指令接口
    void SendCollect();          // 发送收藏命令
    void SendStop();             // 发送停止命令
    void SendZero();             // 发送归零命令
    void SendTrack(int32_t azimuth, int32_t pitch, uint8_t maxAcc);            // 发送跟踪命令
    void SendFollow(int32_t azimuth, int32_t pitch);                           // 发送随动命令 
    void SendScan(int16_t centerAzi, uint16_t scanRange, uint16_t scanSpeed);  // 发送扫描命令
    void SendMemTrack();         // 发送记忆跟踪命令
    void SendVersionQuery();     // 发送版本查询命令

    // 发送/接收消息
    void RecvMsg();             // 接收消息
    void SendMsg();             // 发送消息
    void recvSFStateThread();
    void updateSFState();
    void printServoStatus();

    std::mutex recv_mtx;

private:
    uint8_t CalcXOR(uint8_t* data, int start, int end);  // 异或校验计算

    int sf_fd;
};

#endif // PROTOCOL_SF_H