#include "protocol_env.h"
#include <string.h>
#include <stdio.h>
#include "serialport.h"
#include <mutex>
#include <unistd.h>
#include "protocol_common.h"

extern GlobalReatimeMsg* globalMsg;

ENV_msg::ENV_msg() {
    queryMsg = new ENV_QueryMsg();
    envResponse = new ENV_EnvResponse();
    modelResponse = new ENV_ModelResponse();
    versionResponse = new ENV_VersionResponse();
    serialResponse = new ENV_SerialResponse();
    periodicReport = new ENV_PeriodicReport();
    InitQueryMsg();
}

ENV_msg::~ENV_msg() {
    delete queryMsg;
    delete envResponse;
    delete modelResponse;
    delete versionResponse;
    delete serialResponse;
    delete periodicReport;
}

void ENV_msg::InitQueryMsg() {
    queryMsg->header.header1 = 0xD9;
    queryMsg->header.header2 = 0xD9;
    queryMsg->header.length = 0x06;    // 查询命令固定长度6字节
    queryMsg->header.mode = 0xA1;      // 默认查询模式
    queryMsg->header.command = 0;
    queryMsg->checksum = 0;
}

uint8_t ENV_msg::CalcChecksum(uint8_t* data, uint8_t len) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum ^= data[i];  // 异或校验
    }
    return sum;
}

void ENV_msg::SendMsg(uint8_t* msg, size_t length) {
    //int ret = sp_write(UART_INDEX_ENV, msg, length);
    int ret = send_to_dev(UART_INDEX_HK, (char*)msg, length, 0);
    if (ret <= 0) {
        RK_LOGE("ENV SERIAL SEND ERROR! ERROR CODE:%D", ret);
    }
}

void ENV_msg::RecvMsg() {
    uint8_t buf[256] = { 0 };
	//int recv_from_dev(int dev_index, char* recv_data, int* recv_len, int verbose);
	int totalLen = 0;
	int ret = recv_from_dev(UART_INDEX_HK, (char*)buf, &totalLen, 0);

    // 读取帧头(2字节)
    //int ret = sp_read(UART_INDEX_ENV, buf, 2);
    if (ret <= 0) {
        RK_LOGE("ENV SERIAL READ HEADER ERROR!");
        return;
    }

    if (buf[0] != 0xD9 || buf[1] != 0xD9) {
        RK_LOGE("ENV INVALID FRAME HEADER!");
        return;
    }

    // 读取长度
    uint16_t frameLen = 0;
    if (buf[4] == 0xA2 && buf[5] == E_ENV_PERIODIC_REPORT) {
        // 周期上报模式，读取双字节长度
        frameLen = buf[2] | (buf[3] << 8);  // 低字节在前
    }
    else {
        // 普通响应模式，读取单字节长度
        frameLen = buf[2];
    }

    // 校验和检查
    uint8_t checksum = CalcChecksum(buf, frameLen - 1);
    if (checksum != buf[frameLen - 1]) {
        RK_LOGE("ENV CHECKSUM ERROR!");
        return;
    }

    // 根据命令码解析不同类型的响应
    /*uint8_t cmd = buf[4]; 
    switch (cmd) {
    case E_ENV_QUERY_MODEL:
        ParseModelResponse(buf);
        break;
    case E_ENV_QUERY_VERSION:
        ParseVersionResponse(buf);
        break;
    case E_ENV_QUERY_SERIAL:
        ParseSerialResponse(buf);
        break;
    case E_ENV_QUERY_TEMP:
    case E_ENV_QUERY_PRESSURE:
    case E_ENV_QUERY_HUMIDITY:
    case E_ENV_QUERY_VOLTAGE:
    case E_ENV_QUERY_EXTREMUM:
        ParseEnvResponse(buf);
        break;
    case E_ENV_PERIODIC_REPORT:
        ParsePeriodicReport(buf);
        break;
    default:
        RK_LOGE("ENV UNKNOWN COMMAND: 0x%02X", cmd);
        break;*/
    
}

void ENV_msg::hkRecvThread()
{
    while (true) {
        RecvMsg(); // 接收并处理环控消息
        updateCommonState(); // 更新全局状态
        usleep(10000); // 10ms轮询间隔
    }
}

void ENV_msg::hkQueryThread()
{
    while (true) {
       
        // 周期查询
        void EnablePeriodicReport();                                  // 周期上报


        
        //// 查询温度
        //QueryTemperature();
        //usleep(100000); // 100ms间隔

        //// 查询气压
        //QueryPressure();
        //usleep(100000);

        //// 查询湿度
        //QueryHumidity();
        //usleep(100000);

        //// 查询电压
        //QueryVoltage();
        //usleep(100000);

        //// 每30秒查询一次非关键参数
        //static int counter = 0;
        //if (counter % 30 == 0) {
        //    QueryProductModel();
        //    usleep(100000);
        //    QuerySoftwareVersion();
        //    usleep(100000);
        //    QuerySerialNumber();
        //    usleep(100000);
        //    QueryExtremum();
        //}
        //counter++;

        usleep(500000); // 总查询周期约为1秒
    }
}

void ENV_msg::updateCommonState()
{
    // 周期上报数据更新，包含全部参数
    if (periodicReport != nullptr) {
        globalMsg->m_hkMsg.f_temperature = ParseEnvData(&periodicReport->temperature);
        globalMsg->m_hkMsg.f_pressure = ParseEnvData(&periodicReport->pressure);
        globalMsg->m_hkMsg.f_humidity = ParseEnvData(&periodicReport->humidity);
        globalMsg->m_hkMsg.f_voltage = ParseEnvData(&periodicReport->voltage);
    }
}

// 查询命令实现
void ENV_msg::QueryProductModel() {
    queryMsg->header.command = E_ENV_QUERY_MODEL;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
}

void ENV_msg::QuerySoftwareVersion() {
    queryMsg->header.command = E_ENV_QUERY_VERSION;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
}

void ENV_msg::QuerySerialNumber() {
    queryMsg->header.command = E_ENV_QUERY_SERIAL;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
}

void ENV_msg::QueryTemperature() {
    queryMsg->header.command = E_ENV_QUERY_TEMP;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
}

void ENV_msg::QueryPressure() {
    queryMsg->header.command = E_ENV_QUERY_PRESSURE;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
}

void ENV_msg::QueryHumidity() {
    queryMsg->header.command = E_ENV_QUERY_HUMIDITY;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
}

void ENV_msg::QueryVoltage() {
    queryMsg->header.command = E_ENV_QUERY_VOLTAGE;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
}

void ENV_msg::QueryExtremum() {
    queryMsg->header.command = E_ENV_QUERY_EXTREMUM;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
}

// 周期上报命令实现
void ENV_msg::EnablePeriodicReport() {
    queryMsg->header.mode = 0xA2;  // 切换到周期上报模式
    queryMsg->header.command = E_ENV_PERIODIC_REPORT;
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(ENV_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(ENV_QueryMsg));
    //queryMsg->header.mode = 0xA1;  // 恢复查询模式
}

// 数据解析实现
float ENV_msg::ParseEnvData(const ENV_Data* data) {
    float value = 0.0f;
    if (data) {
        // 处理温度的符号位(最高位为1表示负数)
        if (data->integer & 0x80) {
            value = -((data->integer & 0x7F) + data->decimal / 100.0f);
        }
        else {
            value = data->integer + data->decimal / 100.0f;
        }
    }
    return value;
}

void ENV_msg::ParseVersionInfo(const ENV_VersionInfo* info, char* buf, size_t size) {
    if (info && buf && size >= 16) {
        snprintf(buf, size, "V%d.%d-%d",
            (info->version >> 8) & 0xFF,
            info->version & 0xFF,
            info->date);
    }
}

void ENV_msg::ParseSerialInfo(const ENV_SerialInfo* info, char* buf, size_t size) {
    if (info && buf && size >= 16) {
        snprintf(buf, size, "%04X-%08X",
            info->number,
            info->date);
    }
}

void ENV_msg::ParseEnvResponse(const uint8_t* data) {
    if (data) {
        memcpy(envResponse, data, sizeof(ENV_EnvResponse));
    }
}

void ENV_msg::ParseModelResponse(const uint8_t* data) {
    if (data) {
        memcpy(modelResponse, data, sizeof(ENV_ModelResponse));
    }
}

void ENV_msg::ParseVersionResponse(const uint8_t* data) {
    if (data) {
        memcpy(versionResponse, data, sizeof(ENV_VersionResponse));
    }
}

void ENV_msg::ParseSerialResponse(const uint8_t* data) {
    if (data) {
        memcpy(serialResponse, data, sizeof(ENV_SerialResponse));
    }
}

void ENV_msg::ParsePeriodicReport(const uint8_t* data) {
    if (data) {
        memcpy(periodicReport, data, sizeof(ENV_PeriodicReport));

    }
}