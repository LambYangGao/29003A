#include "self_check.h"
#include <unistd.h>
#include "rk_debug.h"
#include "protocol_power.h"
#include "protocol_sf.h"
#include "protocol_vi.h"
#include "protocol_ir.h"
#include "protocol_picxk.h"
#include "xkMsg.h"
#include "protocol_common.h"
#include "protocol_jgzm.h"
#include "protocol_board_abc.h"
#include "mylog.hpp"

extern POWER_msg* power_msg;
extern Vi_msg* ccdmsg;
extern IR_msg* irmsg;
extern Jgzm_msg* jgzmmsg;
extern SF_msg* sf_msg;
extern GlobalReatimeMsg* globalMsg;
extern XKMsg* xkMsg;
extern BOARDABC_MSG* boardabcMsg;

#define ERROR_THRESHOLD 3

SelfCheck::SelfCheck()
{
}

SelfCheck::~SelfCheck()
{
}

void SelfCheck::bootcheck()
{
    //LOG(INFO) << "Beginning bootcheck procedure...";

    globalMsg->m_senerPowerState.e_Laser_on == 1;
    globalMsg->m_senerPowerState.e_SF_on == 1;
    //LOG(INFO) << "Boot-time self-check completed";
}

void SelfCheck::cyclecheck()
{
    //LOG(INFO) << "Cycle self-check thread started";

    while (1) {
        
		updateGlobalStatus();

        // ÉÏ±¨×Ô¼ì×´Ì¬
		xkMsg->globalSelfCheckStatesUp();

        sleep(30);
    }
}


void SelfCheck::updateGlobalStatus()
{
    bool anyAbnormal =
        globalMsg->m_cycleCheckMsg.e_SocA_state != 0 ||
        globalMsg->m_cycleCheckMsg.e_SocB_state != 0 ||
        globalMsg->m_cycleCheckMsg.e_CCD_state != 0 ||
        globalMsg->m_cycleCheckMsg.e_IR_state != 0 ||
        globalMsg->m_cycleCheckMsg.e_Laser_state != 0 ||
        globalMsg->m_cycleCheckMsg.e_SF_state != 0 ||
        globalMsg->m_cycleCheckMsg.e_Power_state != 0 ;

    if (anyAbnormal) {
        //LOG(WARNING) << "System self-check detected abnormal modules";
    }
    else {
        //LOG(INFO) << "All system modules checked normal";
    }
}