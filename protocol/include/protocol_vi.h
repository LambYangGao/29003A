#ifndef PROTOCOL_VI_H
#define PROTOCOL_VI_H

#include <stdint.h>
#include <vector>
#include <mutex>
#include <cmath>
#include "rk_debug.h"
#include "xuart_api.h"
#include "protocol_common.h"

// 帧头和帧尾
#define HEAD1_VALUE      0xEB    // 发送/接收帧头1
#define HEAD2_TX_VALUE   0x90    // 发送帧头2
#define HEAD2_RX_VALUE   0x80    // 接收帧头2
#define FRAME_LENGTH     0x0C    // 接收帧数据长度
// 发送命令字定义
#define CMD_IDLE         0x00    // 空闲指令（保持通讯，兼备变焦、聚焦停止）
#define CMD_ZOOM_PLUS    0x11    // 变焦+
#define CMD_ZOOM_MINUS   0x12    // 变焦-
#define CMD_ZOOM_POS     0x13    // 变焦至指定位置
#define CMD_EZOOM_OFF    0x14    // 电子变倍关
#define CMD_EZOOM_2X     0x15    // 电子变倍2倍
#define CMD_EZOOM_4X     0x16    // 电子变倍4倍
#define CMD_FOCUS_PLUS   0x21    // 聚焦+
#define CMD_FOCUS_MINUS  0x22    // 聚焦-
#define CMD_FOCUS_POS    0x23    // 聚焦至指定位置
#define CMD_FOCUS_AUTO   0x24    // 一键聚焦
#define CMD_PRESET_SET   0x25    // 设置组合预置位
#define CMD_PRESET_CALL  0x26    // 调用组合预置位
#define CMD_EXPOSURE_AUTO    0x31    // 自动调光
#define CMD_EXPOSURE_MANUAL  0x32    // 手动调光
#define CMD_BRIGHTNESS_PLUS  0x33    // 亮度+（自动调光模式）
#define CMD_BRIGHTNESS_MINUS 0x34    // 亮度-（自动调光模式）
#define CMD_BRIGHTNESS_DEF   0x35    // 亮度默认（自动调光模式）
#define CMD_GAIN_PLUS    0x36    // 增益+（手动调光模式）
#define CMD_GAIN_MINUS   0x37    // 增益-（手动调光模式）
#define CMD_GAIN_SET     0x38    // 设置增益值（手动调光模式，0-240）
#define CMD_SHUTTER_PLUS  0x39    // 电子快门+（手动调光模式）
#define CMD_SHUTTER_MINUS 0x3A    // 电子快门-（手动调光模式）
#define CMD_SHUTTER_SET   0x3B    // 设置电子快门值（手动调光模式，1-20000μs）
#define CMD_FILTER_VIS    0x40    // 可见光滤色片（光学透雾关）
#define CMD_FILTER_NIR    0x41    // 近红外滤色片（光学透雾开）
#define CMD_EDEFOG_OFF    0x42    // 电子透雾关
#define CMD_EDEFOG_ON     0x43    // 电子透雾开
#define CMD_ENHANCE_OFF   0x50    // 图像增强关
#define CMD_ENHANCE_WEAK  0x51    // 图像增强-弱
#define CMD_ENHANCE_MID   0x52    // 图像增强-中
#define CMD_ENHANCE_STRONG 0x53   // 图像增强-强
#define CMD_ENHANCE_CYCLE  0x54   // 图像增强开（三挡循环）
#define CMD_FLIP_NORMAL   0x60    // 图像方向正常
#define CMD_FLIP_H        0x61    // 图像左右翻转
#define CMD_FLIP_V        0x62    // 图像上下翻转
#define CMD_FLIP_MIRROR   0x63    // 图像镜像
#define CMD_CROSS_OFF     0x70    // 图像十字丝关闭
#define CMD_CROSS_ON      0x71    // 图像十字丝开启
#define CMD_SELF_CHECK    0xA0    // 自检

#pragma pack(push)
#pragma pack(1)

// 发送数据帧
typedef struct {
    uint8_t HEAD1;        // 帧头1: 0xEB
    uint8_t HEAD2;        // 帧头2: 0x90
    uint8_t CMD;          // 控制字
    uint8_t DATA_LOW;     // 复用数据低字节
    uint8_t DATA_HIGH;    // 复用数据高字节
    uint8_t PRESET_ID;    // 组合预置位编号(1-20)
    uint8_t RESERVED;     // 预留，填0
    uint8_t CHECKSUM;     // 校验和（字节0-6的和取低8位）
} MSG_TX;

// 接收数据帧
typedef struct {
    uint8_t HEAD1;        // 帧头1: 0xEB
    uint8_t HEAD2;        // 帧头2: 0x80
    uint8_t LENGTH;       // 帧长: 0x0C
    uint8_t STATUS1;      // 功能状态反馈1
    uint8_t STATUS2;      // 功能状态反馈2
    uint8_t SELF_CHECK;   // 自检状态
    uint8_t FOCAL_LOW;    // 焦距值低8位
    uint8_t FOCAL_HIGH;   // 焦距值高8位
    uint8_t FOCUS_LOW;    // 聚焦位置值低8位
    uint8_t FOCUS_HIGH;   // 聚焦位置值高8位
    uint8_t GAIN;         // 当前增益值(0-240)
    uint8_t EXPOSURE_LOW; // 当前曝光值低8位(1-20000μs)
    uint8_t EXPOSURE_HIGH;// 当前曝光值高8位
    uint8_t BRIGHTNESS;   // 当前亮度值(1-255)
    uint8_t CHECKSUM;     // 校验和（字节0-13的和取低8位）
} MSG_RX;

// 功能状态反馈1
typedef struct {
    uint8_t zooming       : 1;  // B0: 1-正在变焦 0-不在变焦
    uint8_t focusing      : 1;  // B1: 1-正在聚焦 0-不在聚焦
    uint8_t filter_moving : 1;  // B2: 1-正在调滤色片 0-不在调节滤色片
    uint8_t ezoom_on      : 1;  // B3: 1-电子变倍开 0-电子变倍关闭
    uint8_t self_checking : 1;  // B4: 1-正在自检 0-不在自检
    uint8_t manual_exp    : 1;  // B5: 1-手动调光 0-自动调光
    uint8_t preset_exec   : 1;  // B6: 1-预置位调用执行中 0-预置位调用空闲中
    uint8_t cross_on      : 1;  // B7: 1-十字丝开启 0-十字丝关闭
} STATUS1_BITS;

// 功能状态反馈2
typedef struct {
    uint8_t filter_state  : 2;  // B0-B1: 00-可见光 01-近红外
    uint8_t edefog_state  : 2;  // B2-B3: 00-电子透雾关 01-电子透雾开
    uint8_t enhance_state : 2;  // B4-B5: 00-关 01-弱 10-中 11-强
    uint8_t ezoom_state   : 2;  // B6-B7: 00-关 01-2× 10-4×
} STATUS2_BITS;

// 自检状态位
typedef struct {
    uint8_t zoom_fault    : 1;  // B0: 1-变焦故障 0-变焦正常
    uint8_t focus_fault   : 1;  // B1: 1-聚焦故障 0-聚焦正常
    uint8_t reserved      : 1;  // B2: 预留，填0
    uint8_t camera_fault  : 1;  // B3: 1-相机故障 0-相机正常
    uint8_t control_fault : 1;  // B4: 1-控制模块故障 0-控制模块正常
    uint8_t filter_fault  : 2;  // B5-B6: 00-正常 11-滤色片故障
    uint8_t check_ok      : 1;  // B7: 1-自检正常 0-自检不正常
} SELFCHECK_BITS;

#pragma pack(pop)


class Vi_msg {
public:
    Vi_msg();
    ~Vi_msg();

    // ========== 变焦控制 ==========
    void zoomStop();                        // 停止变焦
    void zoomTele();                        // 变焦+（放大）
    void zoomWide();                        // 变焦-（缩小）
    void zoomDirect(float focal_length);    // 变焦至指定焦距（实际值的10倍）

    // ========== 电子变倍控制 ==========
    void setElectronicZoom(uint8_t mode);   // 设置电子变倍（0-关 1-2倍 2-4倍）

    // ========== 聚焦控制 ==========
    void focusStop();                       // 停止聚焦
    void focusFar();                        // 聚焦+（远）
    void focusNear();                       // 聚焦-（近）
    void focusDirect(uint16_t position);    // 聚焦至指定位置
    void focusOnePush();                    // 一键聚焦

    // ========== 预置位控制 ==========
    void setPresetPosition(uint8_t id);     // 设置组合预置位（1-20）
    void callPresetPosition(uint8_t id);    // 调用组合预置位（1-20）

    // ========== 曝光控制 ==========
    void setExposureAuto();                 // 自动调光（默认）
    void setExposureManual();               // 手动调光
    void setBrightnessPlus();               // 亮度+（自动调光模式）
    void setBrightnessMinus();              // 亮度-（自动调光模式）
    void setBrightnessDefault();            // 亮度默认（自动调光模式）
    void setGainPlus();                     // 增益+（手动调光模式）
    void setGainMinus();                    // 增益-（手动调光模式）
    void setGainValue(uint8_t value);       // 设置增益值（0-240，手动调光模式）
    void setShutterPlus();                  // 电子快门+（手动调光模式）
    void setShutterMinus();                 // 电子快门-（手动调光模式）
    void setShutterValue(uint16_t value);   // 设置电子快门值（1-20000μs，手动调光模式）

    // ========== 滤色片控制 ==========
    void setFilterVisible();                // 可见光滤色片（光学透雾关）
    void setFilterNearInfrared();           // 近红外滤色片（光学透雾开）

    // ========== 透雾控制 ==========
    void setElectronicDefogOff();           // 电子透雾关
    void setElectronicDefogOn();            // 电子透雾开

    // ========== 图像增强控制 ==========
    void setImageEnhanceOff();              // 图像增强关（默认）
    void setImageEnhanceWeak();             // 图像增强-弱
    void setImageEnhanceMid();              // 图像增强-中
    void setImageEnhanceStrong();           // 图像增强-强
    void setImageEnhanceCycle();            // 图像增强开（三挡循环）

    // ========== 图像翻转控制 ==========
    void setPictureFlipNormal();            // 图像方向正常（自动保存配置）
    void setPictureFlipH();                 // 图像左右翻转（自动保存配置）
    void setPictureFlipV();                 // 图像上下翻转（自动保存配置）
    void setPictureFlipMirror();            // 图像镜像（自动保存配置）

    // ========== 十字丝控制 ==========
    void setCrossHairOff();                 // 图像十字丝关闭
    void setCrossHairOn();                  // 图像十字丝开启

    // ========== 自检控制 ==========
    void startSelfCheck();                  // 启动自检

    // ========== 状态获取 ==========
    void updateCommonState();               // 更新全局状态
    void recvCCDThread();                   // 实时接收状态信息线程
    void queryStateThread();                // 实时状态查询线程
    void parseRxMsg(uint8_t* buf, int len); // 解析接收到的状态数据

    // ========== 跟焦曲线管理接口 ==========
    // 加载配置文件
    void LoadFocusCurve(const char* configPath);
    // 保存当前曲线到文件
    void SaveFocusCurve();
    // 根据当前变倍值(zoom)获取最佳聚焦值(focus)，查表+插值
    uint16_t GetBestFocus(uint16_t zoom_pos);
    // 执行自动标定流程 (阻塞式，在独立线程或维护模式下调用)
    void RunAutoCalibration();

    // ========== 工具函数 ==========
    void initSetting();                     // 初始化默认设置
    uint8_t calculateChecksum(const uint8_t* data, size_t len); // 计算校验和
    uint16_t makeShort(uint8_t high, uint8_t low);   // 组合两个字节为short
    void splitShort(uint16_t value, uint8_t& high, uint8_t& low); // 拆分short为两个字节

public:
    MSG_TX* msgTx;                          // 发送消息
    MSG_RX* msgRx;                          // 接收消息

    // 当前状态变量
    bool m_zooming;                         // 正在变焦
    bool m_focusing;                        // 正在聚焦
    bool m_filter_moving;                   // 正在调滤色片
    bool m_ezoom_on;                        // 电子变倍开启
    bool m_self_checking;                   // 正在自检
    bool m_manual_exp;                      // 手动调光模式
    bool m_preset_exec;                     // 预置位调用执行中
    bool m_cross_on;                        // 十字丝开启
    
    uint8_t m_filter_state;                 // 滤色片状态（0-可见光 1-近红外）
    uint8_t m_edefog_state;                 // 电子透雾状态（0-关 1-开）
    uint8_t m_enhance_state;                // 图像增强状态（0-关 1-弱 2-中 3-强）
    uint8_t m_ezoom_state;                  // 电子变倍状态（0-关 1-2× 2-4×）
    
    bool m_zoom_fault;                      // 变焦故障
    bool m_focus_fault;                     // 聚焦故障
    bool m_camera_fault;                    // 相机故障
    bool m_control_fault;                   // 控制模块故障
    uint8_t m_filter_fault;                 // 滤色片故障（0-正常 3-故障）
    bool m_check_ok;                        // 自检正常
    
    float m_focal_length;                   // 当前焦距值（实际值）
    uint16_t m_focus_position;              // 当前聚焦位置
    uint16_t m_zoom_position;               // 当前变倍位置 (应用Zoom Step，此处暂用焦距值作为Key)
    uint8_t m_gain_value;                   // 当前增益值（0-240）
    uint16_t m_exposure_value;              // 当前曝光值（1-20000μs）
    uint8_t m_brightness_value;             // 当前亮度值（1-255）

    std::mutex recv_mtx;                    // 接收数据互斥锁

private:
    void sendMsg(const uint8_t* msg, size_t length); // 发送消息
    void initMsg();                         // 初始化消息

    // 跟焦曲线相关
    std::string m_curveConfigPath;
    std::map<int, int> m_focusCurve; // Key: ZoomPos, Value: FocusPos
    std::mutex m_curveMtx;
};

#endif // PROTOCOL_VI_H