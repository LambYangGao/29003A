#include "protocol_power.h"
#include <string.h>
#include <stdio.h>
#include "serialport.h"
#include <mutex>
#include <unistd.h>
#include "protocol_common.h"

#include <sys/stat.h>     
#include <errno.h>         
#include <new>            


extern GlobalReatimeMsg* globalMsg;

POWER_msg::POWER_msg() {

    //RK_LOGE("POWER_msg start");
    power_fd=uartextend_init(115200, 0, RK_TO_OUT_POWER_UART);
    //printf("power_fd %d \n", power_fd);
    //power_fd = -1;
    queryMsg = new POWER_QueryMsg();
    controlMsg = new POWER_ControlMsg();
    response = new POWER_Response();
    InitQueryMsg();
    InitControlMsg();

}

POWER_msg::~POWER_msg() {
    delete queryMsg;
    delete controlMsg;
    delete response;
}

void POWER_msg::InitQueryMsg() {
    queryMsg->header.header1 = 0xD7;
    queryMsg->header.header2 = 0xD7;
    queryMsg->header.length = 0x06;
    queryMsg->header.mode = 0xA1;  // 默认查询模式
    queryMsg->header.command = E_POWER_QUERY;
    queryMsg->checksum = 0;
}

void POWER_msg::InitControlMsg() {
    controlMsg->header.header1 = 0xD7;
    controlMsg->header.header2 = 0xD7;
    controlMsg->header.length = 0x0A;
    controlMsg->header.mode = 0xA2;  // 控制模式
    controlMsg->header.command = E_POWER_CONTROL;
    controlMsg->payload_power = E_POWER_OFF;
    controlMsg->servo_power = E_POWER_OFF;
    controlMsg->sar_proc_power = E_POWER_OFF;
    controlMsg->sar_rf_power = E_POWER_OFF;
    controlMsg->checksum = 0;
}

uint8_t POWER_msg::CalcChecksum(uint8_t* data, uint8_t len) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum ^= data[i];  // 异或校验
    }
    return sum;
}

void POWER_msg::SendMsg(uint8_t* msg, size_t length) {

    //for (int i = 0; i < length; i++)
    //{
    //    printf(" %02x ", msg[i]);
    //}
    //printf("\n");

    //int ret = sp_write(UART_INDEX_OUT_POWER, msg, length);
    const void* buf = (void*)msg;
    int ret = uartextend_write(power_fd, buf, length);
    if (ret < 0) {
        RK_LOGE("POWER SERIAL SEND ERROR! ERROR CODE:%d", ret);
    }
}

void POWER_msg::RecvMsg() {
    uint8_t buf[256] = { 0 };
	uint64_t last_recv_time = 0;
	uint64_t current_time;
	const int64_t MAX_TIME_DIFF = 5000;  // 5秒
    // 等待数据到达
    int timeout = 100; // 100ms 超时
    if (uartextend_wait_interrupt(power_fd, timeout) > 0) {
        int ret = uartextend_read(power_fd, buf, sizeof(buf));
        if (ret <= 0) {
            RK_LOGE("POWER serial receive error! code:%d", ret);
            return;
        }

        if (ret < 2) {
            RK_LOGD("Insufficient data received: %d bytes", ret);
            return;
        }

		current_time = getCurTickCount();
		if (last_recv_time != 0)
		{
			int64_t time_diff = current_time - last_recv_time;
			if (time_diff > MAX_TIME_DIFF)
			{
                globalMsg->m_cycleCheckMsg.e_Power_state = 1;
			}
			else
			{
                globalMsg->m_cycleCheckMsg.e_Power_state = 0;

			}
		}
		last_recv_time = current_time;

		//RK_LOGE("POWER serial receive code:0x%02X  0x%02X", buf[1], buf[2]);
        // 在所有接收到的数据中查找有效帧
        if (ret >= sizeof(POWER_Response)) {
            for (int i = 0; i < ret - sizeof(POWER_Response) + 1; i++) {
                // 查找帧头
                if (buf[i] == 0xD7 && buf[i + 1] == 0xD7) {
                    // 验证校验和
                    uint8_t checksum = CalcChecksum(&buf[i], sizeof(POWER_Response) - 1);
                    if (checksum == buf[i + sizeof(POWER_Response) - 1]) {
                        ParseResponse(&buf[i]);
                        return;
                    }
                    else {
                        RK_LOGE("POWER checksum error: Calculated=0x%02X, Received=0x%02X",
                            checksum, buf[i + sizeof(POWER_Response) - 1]);
                    }
                }
            }
            RK_LOGE("POWER invalid frame or checksum error");
        }
    }
    else {
        RK_LOGD("No data received within timeout");
    }
}

void POWER_msg::queryPowerThread() {
    while (true) {
        // 查询电源状态
        QueryStatus();

        // 接收数据
        RecvMsg();

        // 解析响应并更新全局状态
        //updatePowerState();

        // 查询间隔
        usleep(1000000); // 1秒间隔
    }
}

void POWER_msg::updatePowerState()
{
    if (response != nullptr) {

       
    }
}

void POWER_msg::QueryStatus() {
    queryMsg->checksum = CalcChecksum((uint8_t*)queryMsg, sizeof(POWER_FrameHeader));
    SendMsg((uint8_t*)queryMsg, sizeof(POWER_QueryMsg));
}

void POWER_msg::ControlPower(bool payload_on, bool servo_on, bool sar_proc_on, bool sar_rf_on) {
    controlMsg->payload_power = payload_on ? E_POWER_ON : E_POWER_OFF;
    controlMsg->servo_power = servo_on ? E_POWER_ON : E_POWER_OFF;
    controlMsg->sar_proc_power = sar_proc_on ? E_POWER_ON : E_POWER_OFF;
    controlMsg->sar_rf_power = sar_rf_on ? E_POWER_ON : E_POWER_OFF;

    controlMsg->checksum = CalcChecksum((uint8_t*)controlMsg, sizeof(POWER_ControlMsg) - 1);
    SendMsg((uint8_t*)controlMsg, sizeof(POWER_ControlMsg));
}

float POWER_msg::ParseData(const POWER_Data* data, bool isTemperature) {
    float value = 0.0f;
    if (data) {
        if (isTemperature && (data->integer & 0x80)) {
            // 温度负数处理
            value = -((data->integer & 0x7F) + data->decimal / 100.0f);
        }
        else {
            value = data->integer + data->decimal / 100.0f;
        }
    }
    return value;
}

void POWER_msg::ParseResponse(uint8_t* data) {
    if (data) {
        memcpy(response, data, sizeof(POWER_Response));
        ParseResponseData();
    }
}

void POWER_msg::ParseResponseData() {
    // 解析所有电压电流数据
    float payload_voltage = ParseData(&response->payload_voltage);
    float servo_voltage = ParseData(&response->servo_voltage);
    float servo_current = ParseData(&response->servo_current);
    float proc_voltage = ParseData(&response->proc_voltage);
    float proc_current = ParseData(&response->proc_current);
    float fanA_voltage = ParseData(&response->fanA_voltage);
    float fanB_voltage = ParseData(&response->fanB_voltage);
    float sar_proc_voltage = ParseData(&response->sar_proc_voltage);
    float sar_proc_current = ParseData(&response->sar_proc_current);
    float sar_rf_voltage = ParseData(&response->sar_rf_voltage);
    float sar_rf_current = ParseData(&response->sar_rf_current);
    float temperature = ParseData(&response->temperature, true);  
	//printf("POWER proc_voltage:%.2fV\n", proc_voltage);
	//printf("POWER temperature:%.2f℃\n", temperature);

    globalMsg->m_cycleCheckMsg.temperature = temperature;
    globalMsg->m_cycleCheckMsg.proc_voltage = proc_voltage;

	//uint8_t buf[256] = { 0 };
	//size_t len = sizeof(POWER_Response);
	//int totalLen = 0;
	//int ret = recv_from_dev(UART_INDEX_CCDIR_POWER, (char*)buf, &totalLen, 0);
	//if (ret <= 0) {
	//	RK_LOGE("POWER serial receive error! code:%d", ret);
	//	return;
	//}

	//if (ret < 2) {
	//	RK_LOGD("Insufficient data received: %d bytes", ret);
	//	return;
	//}
	//// 在所有接收到的数据中查找有效帧
	//if (ret >= sizeof(POWER_Response)) {
	//	for (int i = 0; i < ret - sizeof(POWER_Response) + 1; i++) {
	//		// 查找帧头
	//		if (buf[i] == 0xD7 && buf[i + 1] == 0xD7) {
	//			// 验证校验和
	//			uint8_t checksum = CalcChecksum(&buf[i], sizeof(POWER_Response) - 1);
	//			if (checksum == buf[i + sizeof(POWER_Response) - 1]) {
	//				ParseResponse(&buf[i]);
	//				return;
	//			}
	//			else {
	//				RK_LOGE("POWER checksum error: Calculated=0x%02X, Received=0x%02X",
	//					checksum, buf[i + sizeof(POWER_Response) - 1]);
	//			}
	//		}
	//	}
	//	RK_LOGE("POWER invalid frame or checksum error");
	//}
	//else {
	//	RK_LOGD("No data received within timeout");
	//}
}