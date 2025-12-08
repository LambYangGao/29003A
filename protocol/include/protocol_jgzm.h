#ifndef PROTOCOL_JGZM_H
#define PROTOCOL_JGZM_H
#include <stdint.h>
#include "xuart_api.h"
#include "protocol_common.h"
#include "rk_debug.h"

#pragma pack(push)
#pragma pack(1)

// 帧头帧尾定义
#define FRAME_HEADER     0x55    // 控制指令帧头
#define FRAME_TAIL       0xAA    // 控制指令帧尾
#define DATA_HEADER      0xAA    // 常规数据帧头
#define DATA_TAIL        0x55    // 常规数据帧尾
#define QUERY_HEADER     0xCC    // 查询数据帧头
#define QUERY_TAIL       0x55    // 查询数据帧尾

// 指令类型
typedef enum JGZM_CMD_TYPE {
    JGZM_SELF_CHECK = 0xF0,      // 自检指令
    JGZM_SINGLE_RANGING = 0xF2,  // 单次测距
    JGZM_CONT_RANGING = 0xF3,    // 连续测距
    JGZM_DIST_GATE_SET = 0xF4,   // 距离选通设置
    JGZM_BAUD_SET = 0xF6,        // 波特率设置
    JGZM_QUERY = 0xF8,           // 查询指令
    JGZM_STOP_RANGING = 0xF3     // 停止测距(与连续测距同命令字，参数不同)
} JGZM_CMD_TYPE;

// 频率类型定义
typedef enum FREQ_TYPE {
    FREQ_STOP = 0x00,  // 停止
    FREQ_1HZ = 0x01,   // 1Hz
    FREQ_5HZ = 0x02,   // 5Hz
    FREQ_2HZ = 0x03,   // 2Hz
    FREQ_3HZ = 0x04,   // 3Hz
    FREQ_4HZ = 0x05    // 4Hz
} FREQ_TYPE;

// 状态字节定义
typedef enum STATUS_TYPE {
    STATUS_SINGLE = 0xF2,  // 单次测距
    STATUS_1HZ = 0xF3,     // 1Hz
    STATUS_2HZ = 0xF6,     // 2Hz
    STATUS_3HZ = 0xF7,     // 3Hz
    STATUS_4HZ = 0xF8,     // 4Hz
    STATUS_5HZ = 0xF4,     // 5Hz
    STATUS_STOP = 0xF5     // 停止
} STATUS_TYPE;

// 自检结果定义
typedef enum SELF_CHECK_RESULT {
    SELF_CHECK_DEFAULT = 0x00,   // 查询默认值
    SELF_CHECK_FAULT = 0xFF,     // 激光器出光异常
    SELF_CHECK_NORMAL = 0x03     // 激光器出光正常
} SELF_CHECK_RESULT;

// 工作状态定义
typedef enum WORK_STATUS {
    WORK_HIGH_INTENSITY = 0x00,  // 高强度工作模式
    WORK_NORMAL = 0x01           // 正常工作模式
} WORK_STATUS;

// 控制指令帧
typedef struct JGZM_CONTROL_MSG {
    uint8_t header;     // 帧头 0x55
    uint8_t cmd;        // 指令字节
    uint8_t param1;     // 参数1
    uint8_t param2;     // 参数2
    uint8_t checksum;   // 校验和(2-4字节异或)
    uint8_t tail;       // 帧尾 0xAA
} JGZM_CONTROL_MSG;

// 常规数据帧
typedef struct JGZM_DATA_MSG {
    uint8_t header;           // 帧头 0xAA
    uint8_t target_num;       // 目标个数
    uint8_t target1_dis_h;    // 首目标距离高字节
    uint8_t target1_dis_l;    // 首目标距离低字节
    uint8_t target1_dis_dec;  // 首目标距离小数位(×100)
    uint8_t target2_dis_h;    // 末目标距离高字节
    uint8_t target2_dis_l;    // 末目标距离低字节
    uint8_t target2_dis_dec;  // 末目标距离小数位(×100)
    uint8_t status;           // 测距状态字节
    uint8_t laser_num_hh;     // 出光次数最高字节
    uint8_t laser_num_h;      // 出光次数高字节
    uint8_t laser_num_l;      // 出光次数低字节
    uint8_t laser_num_ll;     // 出光次数最低字节
    uint8_t checksum;         // 校验和(2-13字节异或)
    uint8_t tail;             // 帧尾 0x55
} JGZM_DATA_MSG;

// 查询数据帧
typedef struct JGZM_QUERY_MSG {
    uint8_t header;           // 帧头 0xCC
    uint8_t backup;           // 备用
    uint8_t temp;             // 环境温度(8位有符号数,℃)
    uint8_t self_exam;        // 自检信息字节
    uint8_t work_status;      // 高强度工作状态字节
    uint8_t diss_high;        // 距离选通值高字节
    uint8_t diss_low;         // 距离选通值低字节
    uint8_t baud_high;        // 波特率高字节(×100)
    uint8_t baud_low;         // 波特率低字节(×100)
    uint8_t laser_num_hh;     // 出光次数最高字节
    uint8_t laser_num_h;      // 出光次数高字节
    uint8_t laser_num_l;      // 出光次数低字节
    uint8_t laser_num_ll;     // 出光次数最低字节
    uint8_t checksum;         // 校验和(2-13字节异或)
    uint8_t tail;             // 帧尾 0x55
} JGZM_QUERY_MSG;

#pragma pack(pop)

class Jgzm_msg {
public:
    Jgzm_msg();
    ~Jgzm_msg();
    
    void init();

    // 是否有测距值标志位
    bool m_bhaveDisValue = false;

    // 实现全局激光测照器信息的更新
    void updateCommonState();
    
    // 基本控制命令
    void sendStopCmd();                                         // 停止测距
    void sendSingleRangingCmd();                                // 单次测距
    void sendContRangingCmd(uint8_t freq, uint16_t workTime);   // 连续测距(1Hz/5Hz)
    
    // 系统配置命令
    void setBaudrate(uint16_t baudrate);                        // 设置波特率
    void setDistGate(uint16_t distance);                        // 设置距离选通
    
    // 状态查询命令
    void sendSelfCheckCmd();                                    // 自检
    void sendQueryCmd();                                        // 查询状态
    
    // 数据解析和状态查询
    void parseDataMsg(uint8_t* data, int len);                  // 解析常规数据
    void parseQueryMsg(uint8_t* data, int len);                 // 解析查询数据
    bool verifyChecksum(uint8_t* data, int len);                // 校验和验证
    
    // 常规数据获取接口
    uint8_t getTargetNum();                                     // 获取目标个数
    uint16_t getTarget1Distance();                              // 获取首目标距离整数部分
    uint8_t getTarget1DistanceDec();                            // 获取首目标距离小数部分
    float getTarget1DistanceFloat();                            // 获取首目标距离(浮点)
    uint16_t getTarget2Distance();                              // 获取末目标距离整数部分
    uint8_t getTarget2DistanceDec();                            // 获取末目标距离小数部分
    float getTarget2DistanceFloat();                            // 获取末目标距离(浮点)
    uint8_t getRangingStatus();                                 // 获取测距状态
    uint32_t getLaserCount();                                   // 获取激光打光次数
    bool isDistanceValid();                                     // 距离数据是否有效
    
    // 查询数据获取接口
    int8_t getEnvTemperature();                                 // 获取环境温度
    uint8_t getSelfCheckResult();                               // 获取自检结果
    bool isSelfCheckNormal();                                   // 自检是否正常
    uint8_t getWorkStatus();                                    // 获取工作状态
    bool isNormalWorkMode();                                    // 是否正常工作模式
    uint16_t getDistGateValue();                                // 获取距离选通值
    uint16_t getBaudrateValue();                                // 获取波特率值
    
    // 消息处理
    void processDataMessage(uint8_t* msgBuffer);                // 处理常规数据消息
    void processQueryMessage(uint8_t* msgBuffer);               // 处理查询数据消息
    void printDistanceInfo();                                   // 打印测距信息
    void printQueryInfo();                                      // 打印查询信息
    
    // 线程函数
    void recvJGZMThread();                                      // 接收线程
    void queryJGZMStateThread();                                // 查询线程

private:
    JGZM_CONTROL_MSG* controlMsg;   // 控制指令
    JGZM_DATA_MSG* dataMsg;         // 常规数据
    JGZM_QUERY_MSG* queryMsg;       // 查询数据
    
    void calcChecksum(JGZM_CONTROL_MSG* msg);  // 计算校验和
    void sendMsg(uint8_t* data, int len);      // 发送数据
    void RecvMsg();                            // 接收数据
    
    // 数据转换
    uint16_t makeWord(uint8_t high, uint8_t low);
    uint32_t makeDWord(uint8_t hh, uint8_t h, uint8_t l, uint8_t ll);
    void splitWord(uint16_t word, uint8_t& high, uint8_t& low);
};

#endif // PROTOCOL_JGZM_H