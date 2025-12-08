#include "protocol_jgzm.h"
#include <string.h>
#include <stdio.h>
#include "serialport.h"

extern GlobalReatimeMsg* globalMsg;

Jgzm_msg::Jgzm_msg() {
    controlMsg = new JGZM_CONTROL_MSG();
    dataMsg = new JGZM_DATA_MSG();
    queryMsg = new JGZM_QUERY_MSG();
    init();
}

Jgzm_msg::~Jgzm_msg() {
    delete controlMsg;
    controlMsg = nullptr;

    delete dataMsg;
    dataMsg = nullptr;

    delete queryMsg;
    queryMsg = nullptr;
}

void Jgzm_msg::init() {
    memset(controlMsg, 0, sizeof(JGZM_CONTROL_MSG));
    controlMsg->header = FRAME_HEADER;
    controlMsg->tail = FRAME_TAIL;
    
    memset(dataMsg, 0, sizeof(JGZM_DATA_MSG));
    dataMsg->header = DATA_HEADER;
    dataMsg->tail = DATA_TAIL;
    
    memset(queryMsg, 0, sizeof(JGZM_QUERY_MSG));
    queryMsg->header = QUERY_HEADER;
    queryMsg->tail = QUERY_TAIL;
}

void Jgzm_msg::calcChecksum(JGZM_CONTROL_MSG* msg) {
    // 控制指令校验和：2-4字节异或
    msg->checksum = msg->cmd ^ msg->param1 ^ msg->param2;
}

bool Jgzm_msg::verifyChecksum(uint8_t* data, int len) {
    // 数据帧校验：2-13字节异或
    if (len < 15) return false;
    uint8_t calc = 0;
    for (int i = 1; i < 13; i++) {
        calc ^= data[i];
    }
    return (calc == data[13]);
}

void Jgzm_msg::sendMsg(uint8_t* data, int len) {
    int ret = send_to_dev(UART_INDEX_JGCZ, (char*)data, len, 0);
    if (ret < 0) {
        printf("JGZM serial send error! code:%d\n", ret);
    }
}

void Jgzm_msg::sendSingleRangingCmd() {
    // 安全检查
    if (IsInForbiddenZone()) {
        RK_LOGE("[SAFETY] Laser blocked! Current Azimuth: %.2f is in forbidden zone.", globalMsg->m_sfMsg.f_Azi_value);
        return;
    }
    
    controlMsg->cmd = JGZM_SINGLE_RANGING;
    controlMsg->param1 = 0x00;
    controlMsg->param2 = 0x00;
    calcChecksum(controlMsg);
    
    sendMsg((uint8_t*)controlMsg, sizeof(JGZM_CONTROL_MSG));
    printf("[CMD] Single ranging sent\n");
}

void Jgzm_msg::sendContRangingCmd(uint8_t freq, uint16_t workTime) {
    // 安全检查
    if (IsInForbiddenZone()) {
        RK_LOGE("[SAFETY] Laser blocked! Current Azimuth: %.2f is in forbidden zone.", globalMsg->m_sfMsg.f_Azi_value);
        return;
    }
    
    controlMsg->cmd = JGZM_CONT_RANGING;
    controlMsg->param1 = freq;  // FREQ_1HZ/FREQ_2HZ/FREQ_3HZ/FREQ_4HZ/FREQ_5HZ
    controlMsg->param2 = 0x00;
    calcChecksum(controlMsg);
    
    sendMsg((uint8_t*)controlMsg, sizeof(JGZM_CONTROL_MSG));
    //globalMsg->m_laserStateMsg.b_isloopDis = true;
    printf("[CMD] Continuous ranging %dHz sent\n", 
           freq == FREQ_1HZ ? 1 : freq == FREQ_2HZ ? 2 : 
           freq == FREQ_3HZ ? 3 : freq == FREQ_4HZ ? 4 : 5);
}

void Jgzm_msg::sendStopCmd() {
    controlMsg->cmd = JGZM_STOP_RANGING;
    controlMsg->param1 = FREQ_STOP;  // 0x00表示停止
    controlMsg->param2 = 0x00;
    calcChecksum(controlMsg);
    
    sendMsg((uint8_t*)controlMsg, sizeof(JGZM_CONTROL_MSG));
    globalMsg->m_laserStateMsg.b_isloopDis = false;
    printf("[CMD] Stop ranging sent\n");
}

void Jgzm_msg::sendSelfCheckCmd() {
    controlMsg->cmd = JGZM_SELF_CHECK;
    controlMsg->param1 = 0x00;
    controlMsg->param2 = 0x00;
    calcChecksum(controlMsg);
    
    sendMsg((uint8_t*)controlMsg, sizeof(JGZM_CONTROL_MSG));
    printf("[CMD] Self check sent\n");
}

void Jgzm_msg::sendQueryCmd() {
    controlMsg->cmd = JGZM_QUERY;
    controlMsg->param1 = 0x00;
    controlMsg->param2 = 0x00;
    calcChecksum(controlMsg);
    
    sendMsg((uint8_t*)controlMsg, sizeof(JGZM_CONTROL_MSG));
    printf("[CMD] Query sent\n");
}

void Jgzm_msg::setBaudrate(uint16_t baudrate) {
    // 波特率 = 实际值 / 100
    uint16_t baud_value = baudrate / 100;
    
    controlMsg->cmd = JGZM_BAUD_SET;
    splitWord(baud_value, controlMsg->param1, controlMsg->param2);
    
    // 根据波特率设置对应的校验码
    if (baudrate == 115200) {
        controlMsg->checksum = 0x72;
    } else if (baudrate == 57600) {
        controlMsg->checksum = 0xB4;
    } else if (baudrate == 38400) {
        controlMsg->checksum = 0x77;
    } else if (baudrate == 9600) {
        controlMsg->checksum = 0x96;
    } else {
        calcChecksum(controlMsg);  // 其他波特率使用计算的校验和
    }
    
    sendMsg((uint8_t*)controlMsg, sizeof(JGZM_CONTROL_MSG));
    printf("[CMD] Set baudrate to %d sent\n", baudrate);
}

void Jgzm_msg::setDistGate(uint16_t distance) {
    controlMsg->cmd = JGZM_DIST_GATE_SET;
    splitWord(distance, controlMsg->param1, controlMsg->param2);
    calcChecksum(controlMsg);
    
    sendMsg((uint8_t*)controlMsg, sizeof(JGZM_CONTROL_MSG));
    printf("[CMD] Set distance gate to %dm sent\n", distance);
}

void Jgzm_msg::parseDataMsg(uint8_t* data, int len) {
    if (len < sizeof(JGZM_DATA_MSG)) {
        printf("[Error] Data message length invalid: %d\n", len);
        return;
    }
    
    // 验证帧头帧尾
    if (data[0] != DATA_HEADER || data[14] != DATA_TAIL) {
        printf("[Error] Data frame header/tail invalid\n");
        return;
    }
    
    // 验证校验和
    if (!verifyChecksum(data, len)) {
        printf("[Error] Data checksum error\n");
        return;
    }
    
    // 复制数据
    memcpy(dataMsg, data, sizeof(JGZM_DATA_MSG));
    m_bhaveDisValue = true;
}

void Jgzm_msg::parseQueryMsg(uint8_t* data, int len) {
    if (len < sizeof(JGZM_QUERY_MSG)) {
        printf("[Error] Query message length invalid: %d\n", len);
        return;
    }
    
    // 验证帧头帧尾
    if (data[0] != QUERY_HEADER || data[14] != QUERY_TAIL) {
        printf("[Error] Query frame header/tail invalid\n");
        return;
    }
    
    // 验证校验和
    if (!verifyChecksum(data, len)) {
        printf("[Error] Query checksum error\n");
        return;
    }
    
    // 复制数据
    memcpy(queryMsg, data, sizeof(JGZM_QUERY_MSG));
}

// ========== 常规数据获取接口 ==========
uint8_t Jgzm_msg::getTargetNum() {
    return dataMsg->target_num;
}

uint16_t Jgzm_msg::getTarget1Distance() {
    return makeWord(dataMsg->target1_dis_h, dataMsg->target1_dis_l);
}

uint8_t Jgzm_msg::getTarget1DistanceDec() {
    return dataMsg->target1_dis_dec;
}

float Jgzm_msg::getTarget1DistanceFloat() {
    uint16_t dist_int = getTarget1Distance();
    uint8_t dist_dec = getTarget1DistanceDec();
    return (float)dist_int + (float)dist_dec / 100.0f;
}

uint16_t Jgzm_msg::getTarget2Distance() {
    return makeWord(dataMsg->target2_dis_h, dataMsg->target2_dis_l);
}

uint8_t Jgzm_msg::getTarget2DistanceDec() {
    return dataMsg->target2_dis_dec;
}

float Jgzm_msg::getTarget2DistanceFloat() {
    uint16_t dist_int = getTarget2Distance();
    uint8_t dist_dec = getTarget2DistanceDec();
    return (float)dist_int + (float)dist_dec / 100.0f;
}

uint8_t Jgzm_msg::getRangingStatus() {
    return dataMsg->status;
}

uint32_t Jgzm_msg::getLaserCount() {
    return makeDWord(dataMsg->laser_num_hh, dataMsg->laser_num_h, 
                     dataMsg->laser_num_l, dataMsg->laser_num_ll);
}

bool Jgzm_msg::isDistanceValid() {
    // 测距超程或盲区内显示0xFF，即无效状态
    return (dataMsg->target1_dis_h != 0xFF && 
            dataMsg->target1_dis_l != 0xFF &&
            dataMsg->target1_dis_dec != 0xFF);
}

// ========== 查询数据获取接口 ==========
int8_t Jgzm_msg::getEnvTemperature() {
    return (int8_t)queryMsg->temp;
}

uint8_t Jgzm_msg::getSelfCheckResult() {
    return queryMsg->self_exam;
}

bool Jgzm_msg::isSelfCheckNormal() {
    return (queryMsg->self_exam == SELF_CHECK_NORMAL);
}

uint8_t Jgzm_msg::getWorkStatus() {
    return queryMsg->work_status;
}

bool Jgzm_msg::isNormalWorkMode() {
    return (queryMsg->work_status == WORK_NORMAL);
}

uint16_t Jgzm_msg::getDistGateValue() {
    return makeWord(queryMsg->diss_high, queryMsg->diss_low);
}

uint16_t Jgzm_msg::getBaudrateValue() {
    return makeWord(queryMsg->baud_high, queryMsg->baud_low) * 100;
}

void Jgzm_msg::processDataMessage(uint8_t* msgBuffer) {
    parseDataMsg(msgBuffer, sizeof(JGZM_DATA_MSG));
    
    if (m_bhaveDisValue) {
        printDistanceInfo();
    }
}

void Jgzm_msg::processQueryMessage(uint8_t* msgBuffer) {
    parseQueryMsg(msgBuffer, sizeof(JGZM_QUERY_MSG));
    printQueryInfo();
}

void Jgzm_msg::printDistanceInfo() {
    if (!isDistanceValid()) {
        printf("[Distance] Invalid (超程或盲区)\n");
        return;
    }
    
    printf("[Distance] Targets:%d T1:%.2fm T2:%.2fm Status:0x%02X LaserCount:%u\n",
           getTargetNum(),
           getTarget1DistanceFloat(),
           getTarget2DistanceFloat(),
           getRangingStatus(),
           getLaserCount());
}

void Jgzm_msg::printQueryInfo() {
    printf("[Query] Temp:%d℃ SelfCheck:%s WorkMode:%s DistGate:%dm Baud:%d LaserCount:%u\n",
           getEnvTemperature(),
           isSelfCheckNormal() ? "OK" : "Fault",
           isNormalWorkMode() ? "Normal" : "HighIntensity",
           getDistGateValue(),
           getBaudrateValue(),
           makeDWord(queryMsg->laser_num_hh, queryMsg->laser_num_h,
                     queryMsg->laser_num_l, queryMsg->laser_num_ll));
}

void Jgzm_msg::recvJGZMThread() {
    uint8_t buffer[256] = {0};
    uint64_t last_recv_time = 0;
    uint64_t current_time;
    const int64_t MAX_TIME_DIFF = 5000;  // 5秒
    
    while (true) {
        if (globalMsg->m_senerPowerState.e_Laser_on == 1) {
            memset(buffer, 0, 256);
            int totalLen = 0;
            int ret = recv_from_dev(UART_INDEX_JGCZ, (char*)buffer, &totalLen, 0);
            
            if (ret <= 0) {
                printf("JGZM serial receive error! code:%d\n", ret);
                usleep(30000);
                continue;
            }

            if (totalLen < 2) {
                usleep(30000);
                continue;
            }

            current_time = getCurTickCount();
            if (last_recv_time != 0) {
                int64_t time_diff = current_time - last_recv_time;
                if (time_diff > MAX_TIME_DIFF) {
                    globalMsg->m_cycleCheckMsg.e_Laser_state = 1;
                } else {
                    globalMsg->m_cycleCheckMsg.e_Laser_state = 0;
                }
            }
            last_recv_time = current_time;

            // 查找并处理完整帧
            for (int i = 0; i < totalLen - 1; i++) {
                // 查找常规数据帧 (0xAA开头)
                if (buffer[i] == DATA_HEADER && i + sizeof(JGZM_DATA_MSG) <= totalLen) {
                    if (buffer[i + 14] == DATA_TAIL) {
                        processDataMessage(&buffer[i]);
                        i += sizeof(JGZM_DATA_MSG) - 1;
                        continue;
                    }
                }
                
                // 查找查询数据帧 (0xCC开头)
                if (buffer[i] == QUERY_HEADER && i + sizeof(JGZM_QUERY_MSG) <= totalLen) {
                    if (buffer[i + 14] == QUERY_TAIL) {
                        processQueryMessage(&buffer[i]);
                        i += sizeof(JGZM_QUERY_MSG) - 1;
                        continue;
                    }
                }
            }
        }
        
        usleep(30000);
    }
}

void Jgzm_msg::queryJGZMStateThread() {
    while (true) {
        if (globalMsg->m_senerPowerState.e_Laser_on == 1) {
            // 定期发送查询指令
            sendQueryCmd();
            usleep(300000);
        }
        usleep(700000);
    }
}

void Jgzm_msg::updateCommonState() {
    // 距离数据更新
    if (m_bhaveDisValue && isDistanceValid()) {
        globalMsg->m_laserStateMsg.f_Dis_value = getTarget1DistanceFloat();
    }

    // 查询数据更新（如果有的话）
    if (queryMsg->header == QUERY_HEADER) {
        globalMsg->m_laserStateMsg.c_ld_temp = getEnvTemperature();
        
        // 自检状态
        globalMsg->m_laserStateMsg.uc_sysStatus = 
            isSelfCheckNormal() ? 0 : 1;
        
        // 工作模式
        globalMsg->m_laserStateMsg.uc_workModeState = 
            isNormalWorkMode() ? 1 : 0;
    }
    
    // 常规数据状态更新
    if (dataMsg->header == DATA_HEADER) {
        // 根据测距状态判断工作模式
        switch (dataMsg->status) {
            case STATUS_SINGLE:
                globalMsg->m_laserStateMsg.uc_workModeState = 1;
                break;
            case STATUS_1HZ:
            case STATUS_2HZ:
            case STATUS_3HZ:
            case STATUS_4HZ:
            case STATUS_5HZ:
                globalMsg->m_laserStateMsg.uc_workModeState = 2;
                break;
            case STATUS_STOP:
                globalMsg->m_laserStateMsg.uc_workModeState = 0;
                break;
            default:
                break;
        }
    }
}

void Jgzm_msg::RecvMsg() {
    uint8_t buffer[256] = {0};
    int totalLen = 0;
    int ret = recv_from_dev(UART_INDEX_JGCZ, (char*)buffer, &totalLen, 0);

    if (ret <= 0) {
        printf("JGZM serial receive error! code:%d\n", ret);
        return;
    }

    if (totalLen < 2) {
        return;
    }

    // 查找并处理完整帧
    for (int i = 0; i < totalLen - 1; i++) {
        // 查找常规数据帧
        if (buffer[i] == DATA_HEADER && i + sizeof(JGZM_DATA_MSG) <= totalLen) {
            if (buffer[i + 14] == DATA_TAIL) {
                processDataMessage(&buffer[i]);
                return;
            }
        }
        
        // 查找查询数据帧
        if (buffer[i] == QUERY_HEADER && i + sizeof(JGZM_QUERY_MSG) <= totalLen) {
            if (buffer[i + 14] == QUERY_TAIL) {
                processQueryMessage(&buffer[i]);
                return;
            }
        }
    }
}

// ========== 数据转换工具函数 ==========
uint16_t Jgzm_msg::makeWord(uint8_t high, uint8_t low) {
    return ((uint16_t)high << 8) | low;
}

uint32_t Jgzm_msg::makeDWord(uint8_t hh, uint8_t h, uint8_t l, uint8_t ll) {
    return ((uint32_t)hh << 24) | ((uint32_t)h << 16) | 
           ((uint32_t)l << 8) | ll;
}

void Jgzm_msg::splitWord(uint16_t word, uint8_t& high, uint8_t& low) {
    high = (word >> 8) & 0xFF;
    low = word & 0xFF;
}


// 设置禁射区
void Jgzm_msg::SetForbiddenZone(float azi_start, float azi_end) {
    ForbiddenZone zone;
    zone.start_angle = azi_start;
    zone.end_angle = azi_end;
    m_forbiddenZones.push_back(zone);
    RK_LOGI("Set Laser Forbidden Zone: [%.2f, %.2f]", azi_start, azi_end);
}

void Jgzm_msg::ClearForbiddenZones() {
    m_forbiddenZones.clear();
}

// 判断是否在禁射区
bool Jgzm_msg::IsInForbiddenZone() {
    // 获取当前伺服方位角
    float current_azi = globalMsg->m_sfMsg.f_Azi_value;

    for (const auto& zone : m_forbiddenZones) {
        // 处理跨越 +/-180 度的情况
        if (zone.start_angle > zone.end_angle) {
            // 角度 > start 或者 角度 < end
            if (current_azi >= zone.start_angle || current_azi <= zone.end_angle) {
                return true;
            }
        } else {
            // 正常区间：start <= 角度 <= end
            if (current_azi >= zone.start_angle && current_azi <= zone.end_angle) {
                return true;
            }
        }
    }
    return false;
}
