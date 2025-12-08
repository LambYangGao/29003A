#pragma once
#include "protocol_common.h"


class SelfCheck {
public:
    SelfCheck();
    ~SelfCheck();

    // 开机自检
    void bootcheck();

    // 周期自检
    void cyclecheck();

private:

    // 状态更新
    void updateGlobalStatus();

};