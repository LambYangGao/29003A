#include "protocol_sf.h"
#include <string.h>
#include <stdio.h>
#include "serialport.h"
#include <mutex>
#include <unistd.h>
#include "protocol_common.h"
#include <mutex>


extern GlobalReatimeMsg* globalMsg;

#define SF_FRAME_HEAD        0xA5
#define SF_FRAME_TAIL        0x5A
#define SF_VERSION_SYNC1     0x55
#define SF_VERSION_SYNC2     0xAA

// 角度转换因子
#define SF_ANGLE_FACTOR      1000    // 0.001度/LSB
#define SF_SCAN_ANGLE_FACTOR 100     // 0.01度/LSB

SF_msg::SF_msg() {
    sf_fd= uartextend_init(115200, 0, RK_TO_SF_UART);
    statusMsg = new SF_StatusMsg();
    controlMsg = new SF_ControlMsg();
    versionMsg = new SF_VersionMsg();
    InitControlMsg();
}

SF_msg::~SF_msg() {
    delete statusMsg;
    delete controlMsg;
    delete versionMsg;
}

void SF_msg::InitControlMsg() {
    memset(statusMsg, 0, sizeof(SF_StatusMsg));
    memset(controlMsg, 0, sizeof(SF_ControlMsg));
    memset(versionMsg, 0, sizeof(SF_VersionMsg));

    controlMsg->HEAD = SF_FRAME_HEAD;
    controlMsg->LENGTH = 0x0A;
    controlMsg->TAIL = SF_FRAME_TAIL;
}

uint8_t SF_msg::CalcXOR(uint8_t* data, int start, int end) {
    uint8_t checksum = 0;
    for (int i = start; i < end; i++) {
        checksum += data[i];
    }
    return checksum;
}

void SF_msg::CalcChecksum() {
    controlMsg->CHECKSUM = CalcXOR((uint8_t*)controlMsg, 1, sizeof(SF_ControlMsg) - 2);
}

void SF_msg::SendCollect() {
    controlMsg->CMD = E_SF_COLLECT;
    memset(&controlMsg->PARAM1, 0, 8);
    CalcChecksum();
    SendMsg();
}

void SF_msg::SendStop() {
    controlMsg->CMD = E_SF_STOP;
    memset(&controlMsg->PARAM1, 0, 8);
    CalcChecksum();
    SendMsg();
}

void SF_msg::SendZero() {
    controlMsg->CMD = E_SF_ZERO;
    memset(&controlMsg->PARAM1, 0, 8);
    CalcChecksum();
    SendMsg();
}

void SF_msg::SendTrack(int32_t azimuth, int32_t pitch, uint8_t maxAcc) {
    memset(&controlMsg->PARAM1, 0, 8);
    controlMsg->CMD = E_SF_TRACK;

    // 方位角度参数，24位有符号整数，分三字节传输 (-180° ~ +180°)
    if (azimuth > 180 * SF_ANGLE_FACTOR) {
        azimuth = 180 * SF_ANGLE_FACTOR;
    }
    else if (azimuth < -180 * SF_ANGLE_FACTOR) {
        azimuth = -180 * SF_ANGLE_FACTOR;
    }
    controlMsg->PARAM1 = (azimuth >> 16) & 0xFF;
    controlMsg->PARAM2 = (azimuth >> 8) & 0xFF;
    controlMsg->PARAM3 = azimuth & 0xFF;

    // 俯仰角度参数，24位有符号整数，分三字节传输 (-120° ~ +50°)
    if (pitch > 50 * SF_ANGLE_FACTOR) {
        pitch = 50 * SF_ANGLE_FACTOR;
    }
    else if (pitch < -120 * SF_ANGLE_FACTOR) {
        pitch = -120 * SF_ANGLE_FACTOR;
    }
    controlMsg->PARAM4 = (pitch >> 16) & 0xFF;
    controlMsg->PARAM5 = (pitch >> 8) & 0xFF;
    controlMsg->PARAM6 = pitch & 0xFF;

    

    // 跟踪最大加速度参数 (1-20)
    if (maxAcc > 20) {
        maxAcc = 20;
    }
    else if (maxAcc < 1) {
        maxAcc = 1;
    }
    controlMsg->PARAM7 = maxAcc;

    CalcChecksum();
    SendMsg();
}

void SF_msg::SendFollow(int32_t azimuth, int32_t pitch) {
    memset(&controlMsg->PARAM1, 0, 8);
    controlMsg->CMD = E_SF_FOLLOW;

    // 方位角度参数，24位有符号整数 (0° ~ 360°)
    if (azimuth > 360 * SF_ANGLE_FACTOR) {
        azimuth = 360 * SF_ANGLE_FACTOR;
    }
    else if (azimuth < 0) {
        azimuth = 0;
    }
    
    controlMsg->PARAM1 = (azimuth >> 16) & 0xFF;
    controlMsg->PARAM2 = (azimuth >> 8) & 0xFF;
    controlMsg->PARAM3 = azimuth & 0xFF;

    // 俯仰角度参数，24位有符号整数 (-120° ~ +50°)
    if (pitch > 50 * SF_ANGLE_FACTOR) {
        pitch = 50 * SF_ANGLE_FACTOR;
    }
    else if (pitch < -120 * SF_ANGLE_FACTOR) {
        pitch = -120 * SF_ANGLE_FACTOR;
    }
    
    //uint32_t pitch_bits = (uint32_t)pitch;
    controlMsg->PARAM4 = (pitch >> 16) & 0xFF;
    controlMsg->PARAM5 = (pitch >> 8) & 0xFF;
    controlMsg->PARAM6 = pitch & 0xFF;

    CalcChecksum();
    SendMsg();
}

void SF_msg::SendScan(int16_t centerAzi, uint16_t scanRange, uint16_t scanSpeed) {
    memset(&controlMsg->PARAM1, 0, 8);
    controlMsg->CMD = E_SF_SCAN;

    // 方位中心点参数，范围0°～+360° (0.01度/LSB)
    if (centerAzi > 360 * SF_SCAN_ANGLE_FACTOR) {
        centerAzi = 360 * SF_SCAN_ANGLE_FACTOR;
    }
    else if (centerAzi < 0) {
        centerAzi = 0;
    }
    controlMsg->PARAM1 = (centerAzi >> 8) & 0xFF;
    controlMsg->PARAM2 = centerAzi & 0xFF;

    // 扫描幅度参数，范围0～180° (0.01度/LSB)
    if (scanRange > 180 * SF_SCAN_ANGLE_FACTOR) {
        scanRange = 180 * SF_SCAN_ANGLE_FACTOR;
    }
    controlMsg->PARAM3 = (scanRange >> 8) & 0xFF;
    controlMsg->PARAM4 = scanRange & 0xFF;

    // 扫描速度参数，范围0～60°/s (0.01度/秒)
    if (scanSpeed > 60 * SF_SCAN_ANGLE_FACTOR) {
        scanSpeed = 60 * SF_SCAN_ANGLE_FACTOR;
    }
    controlMsg->PARAM5 = (scanSpeed >> 8) & 0xFF;
    controlMsg->PARAM6 = scanSpeed & 0xFF;

    CalcChecksum();
    SendMsg();
}

void SF_msg::SendMemTrack() {
    controlMsg->CMD = E_SF_MEM_TRACK;
    memset(&controlMsg->PARAM1, 0, 8);
    CalcChecksum();
    SendMsg();
}

void SF_msg::SendVersionQuery() {
    controlMsg->CMD = E_SF_VERSION;
    memset(&controlMsg->PARAM1, 0, 8);
    CalcChecksum();
    SendMsg();
}

void SF_msg::RecvMsg() {
    uint8_t buf[256] = { 0 };
	uint64_t last_recv_time = 0;
	uint64_t current_time;
	const int64_t MAX_TIME_DIFF = 5000;  // 5秒
    int timeout = 100;
    if (uartextend_wait_interrupt(sf_fd, timeout) > 0) {
        int ret = uartextend_read(sf_fd, buf, sizeof(buf));
        if (ret <= 0) {
            RK_LOGE("SF serial receive error! code:%d", ret);
            return;
        }

		current_time = getCurTickCount();
		if (last_recv_time != 0)
		{
			int64_t time_diff = current_time - last_recv_time;
			if (time_diff > MAX_TIME_DIFF)
			{
                globalMsg->m_cycleCheckMsg.e_SF_state = 1;
			}
			else
			{
                globalMsg->m_cycleCheckMsg.e_SF_state = 0;

			}
		}
		last_recv_time = current_time;


        // 在所有接收到的数据中查找有效帧
        for (int i = 0; i <= ret - 14; i++) {  // 14是最小帧大小（状态消息）
            if (buf[i] == SF_FRAME_HEAD) {
                if (i + 1 >= ret) continue;  // 没有足够的数据读取长度字节

                uint8_t length = buf[i + 1];
                if (i + length >= ret) continue;  

                if (buf[i + length - 1] != SF_FRAME_TAIL) continue;

                // 验证校验和
                uint8_t checksum = CalcXOR(&buf[i], 1, i + length - 2);
                if (checksum != buf[i + length - 2]) {
                    RK_LOGE("SF 校验和错误! 计算值=0x%02X, 接收值=0x%02X",
                        checksum, buf[i + length - 2]);
                    continue;
                }

                // 处理有效帧
                if (length == 0x0D) {
                    // 检查是否为版本查询响应
                    if (buf[i + 2] == SF_VERSION_SYNC1 && buf[i + 3] == SF_VERSION_SYNC2 &&
                        buf[i + 12] == SF_VERSION_SYNC1 && buf[i + 13] == SF_VERSION_SYNC2) {
                        memcpy(versionMsg, &buf[i], sizeof(SF_VersionMsg));
                    }
                    else {
                        // 状态消息
                        memcpy(statusMsg, &buf[i], sizeof(SF_StatusMsg));
                    }
                    return;  
                }
            }
        }
        RK_LOGE("SF invalid frame or checksum error");
    }
}

void SF_msg::SendMsg() {

    const void* buf = (void*)controlMsg;
    int ret = uartextend_write(sf_fd, buf, sizeof(SF_ControlMsg));
    if (ret < 0) {
        RK_LOGE("SF SERIAL SEND ERROR! ERROR CODE:%d", ret);
    }
    if (ret > 0) {
        RK_LOGE("SF SERIAL SEND success ! :%d", ret);
    }

}

void SF_msg::recvSFStateThread() {
	uint8_t buf[256] = { 0 };
	uint64_t last_recv_time = 0;
	uint64_t current_time;
	const int64_t MAX_TIME_DIFF = 5000;  // 5秒
	int timeout = 100;

    // 打印计数
    int print_counter = 0;
    const int PRINT_INTERVAL = 50;  // 每50次接收打印一次状态

    while (true) {

        if (globalMsg->m_senerPowerState.e_SF_on == 1)
		{
			if (uartextend_wait_interrupt(sf_fd, timeout) > 0) {
				memset(buf, 0, 256);
				int ret = uartextend_read(sf_fd, buf, sizeof(buf));
				if (ret <= 0) {
					RK_LOGE("SF serial receive error! code:%d", ret);
					continue;
				}

				current_time = getCurTickCount();
				if (last_recv_time != 0)
				{
					int64_t time_diff = current_time - last_recv_time;
					if (time_diff > MAX_TIME_DIFF)
					{
						globalMsg->m_cycleCheckMsg.e_SF_state = 1;
					}
					else
					{
						globalMsg->m_cycleCheckMsg.e_SF_state = 0;

					}
				}
				last_recv_time = current_time;


				// 在所有接收到的数据中查找有效帧
				for (int i = 0; i <= ret - 14; i++) {  // 14是最小帧大小（状态消息）
					if (buf[i] == SF_FRAME_HEAD) {
						if (i + 1 >= ret) continue;  // 没有足够的数据读取长度字节

						uint8_t length = buf[i + 1];
						if (i + length >= ret) continue;

						if (buf[i + length - 1] != SF_FRAME_TAIL) continue;

						// 验证校验和
						uint8_t checksum = CalcXOR(&buf[i], 1, i + length - 2);
						if (checksum != buf[i + length - 2]) {
							RK_LOGE("SF 校验和错误! 计算值=0x%02X, 接收值=0x%02X",
								checksum, buf[i + length - 2]);
							continue;
						}

						// 处理有效帧
						if (length == 0x0D) {
							// 检查是否为版本查询响应
							if (buf[i + 2] == SF_VERSION_SYNC1 && buf[i + 3] == SF_VERSION_SYNC2 &&
								buf[i + 12] == SF_VERSION_SYNC1 && buf[i + 13] == SF_VERSION_SYNC2) {
								memcpy(versionMsg, &buf[i], sizeof(SF_VersionMsg));
							}
							else {
								// 状态消息
								memcpy(statusMsg, &buf[i], sizeof(SF_StatusMsg));

                                // 定期打印状态（避免打印过于频繁）
                                print_counter++;
                                if (print_counter >= PRINT_INTERVAL) {
                                    printServoStatus();
                                    print_counter = 0;
                                }
							}
							continue;
						}
					}
				}
				RK_LOGE("SF invalid frame or checksum error");
			}
			// 更新全局状态
			updateSFState();
		}
        
        usleep(10000); // 20ms以匹配伺服上报频率
    }
}

void SF_msg::updateSFState()
{
    if (statusMsg != nullptr) {
        recv_mtx.lock();
        
        // 解析编码器角度值 (19位整数)
        int32_t azimuth = ((statusMsg->AZI_HIGH << 16) | (statusMsg->AZI_MID << 8) | statusMsg->AZI_LOW);
        int32_t pitch = ((statusMsg->PITCH_HIGH << 16) | (statusMsg->PITCH_MID << 8) | statusMsg->PITCH_LOW);

        // 转换为物理角度值，假设编码器值 524288 对应 360度
        float azi_angle = azimuth * 360.0f / 524288.0f;
        if (azi_angle > 180.0f) {
            azi_angle = azi_angle - 360.0f; // 转换为±180度范围
        }

        float pitch_angle = pitch * 360.0f / 524288.0f;
        if (pitch_angle > 180.0f) {
            pitch_angle = pitch_angle - 360.0f; // 转换为±180度范围
        }

        // 更新角度
        globalMsg->m_sfMsg.f_Azi_value = azi_angle;
        globalMsg->m_sfMsg.f_Pitch_value = pitch_angle;

        // 解析陀螺仪角速率 (有符号16位整数)
        int16_t azi_gyro = (statusMsg->AZI_GYRO_H << 8) | statusMsg->AZI_GYRO_L;
        int16_t pitch_gyro = (statusMsg->PITCH_GYRO_H << 8) | statusMsg->PITCH_GYRO_L;

        // 转换为物理角速率，假设±32767对应±300°/s
        float azi_rate = azi_gyro * 300.0f / 32767.0f;
        float pitch_rate = pitch_gyro * 300.0f / 32767.0f;

        globalMsg->m_sfMsg.f_AziRate_value = azi_rate;
        globalMsg->m_sfMsg.f_PitchRate_value = pitch_rate;



        // 更新工作模式到全局变量
        uint8_t workMode = statusMsg->WORK_MODE;
        switch (workMode) {
        case SF_MODE_COLLECT:
            globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_MANUAL;
            break;
        case SF_MODE_STOP:
            globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_MANUAL;
            break;
        case SF_MODE_ZERO:
            globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_MANUAL;
            break;
        case SF_MODE_TRACK:
            globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_AUTO_TRACK;
            break;
        case SF_MODE_FOLLOW:
            globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_PRE_GUIDE;
            break;
        case SF_MODE_AZI_SCAN:
            globalMsg->m_systemControlMsgS.WorkMode = SYS_MODE_UNATTENDED;
            break;
        default:
            break;
        
        }
        recv_mtx.unlock();
    }
}


void SF_msg::printServoStatus() {
    if (statusMsg == nullptr) {
        RK_LOGE("伺服状态消息为空");
        return;
    }
    
    recv_mtx.lock();
    
    printf("\n======== 伺服状态信息 ========\n");
    printf("时间戳: %llu ms\n", getCurTickCount());
    
    // 基本状态信息
    printf("状态字节: 0x%02X ", statusMsg->STATUS);
    if (statusMsg->STATUS == 0) {
        printf("(正常)\n");
    } else {
        printf("(");
        if (statusMsg->STATUS & SF_STATUS_AZI_ENCODER) printf("方位编码器故障 ");
        if (statusMsg->STATUS & SF_STATUS_PITCH_ENCODER) printf("俯仰编码器故障 ");
        if (statusMsg->STATUS & SF_STATUS_AZI_DRIVER) printf("方位驱动器故障 ");
        if (statusMsg->STATUS & SF_STATUS_PITCH_DRIVER) printf("俯仰驱动器故障 ");
        if (statusMsg->STATUS & SF_STATUS_SERVO_BOARD) printf("伺服控制板故障 ");
        if (statusMsg->STATUS & SF_STATUS_GYRO) printf("陀螺仪故障 ");
        printf(")\n");
    }
    
    // 工作模式
    printf("工作模式: 0x%02X ", statusMsg->WORK_MODE);
    switch (statusMsg->WORK_MODE) {
        case SF_MODE_COLLECT: printf("(收藏模式)\n"); break;
        case SF_MODE_STOP: printf("(停止模式)\n"); break;
        case SF_MODE_ZERO: printf("(归零模式)\n"); break;
        case SF_MODE_TRACK: printf("(跟踪模式)\n"); break;
        case SF_MODE_FOLLOW: printf("(随动模式)\n"); break;
        case SF_MODE_AZI_SCAN: printf("(方位扫描模式)\n"); break;
        case SF_MODE_MEM_TRACK: printf("(记忆跟踪模式)\n"); break;
        default: printf("(未知模式)\n"); break;
    }
    
    // 位置信息
    printf("方位角: %.3f° (原始值: %d)\n", 
           globalMsg->m_sfMsg.f_Azi_value,
           ((statusMsg->AZI_HIGH << 16) | (statusMsg->AZI_MID << 8) | statusMsg->AZI_LOW));
    printf("俯仰角: %.3f° (原始值: %d)\n", 
           globalMsg->m_sfMsg.f_Pitch_value,
           ((statusMsg->PITCH_HIGH << 16) | (statusMsg->PITCH_MID << 8) | statusMsg->PITCH_LOW));
    
    // 角速度信息
    printf("方位角速度: %.2f°/s (原始值: %d)\n", 
           globalMsg->m_sfMsg.f_AziRate_value,
           (int16_t)((statusMsg->AZI_GYRO_H << 8) | statusMsg->AZI_GYRO_L));
    printf("俯仰角速度: %.2f°/s (原始值: %d)\n", 
           globalMsg->m_sfMsg.f_PitchRate_value,
           (int16_t)((statusMsg->PITCH_GYRO_H << 8) | statusMsg->PITCH_GYRO_L));
    
    printf("==============================\n\n");
    
    recv_mtx.unlock();
}
