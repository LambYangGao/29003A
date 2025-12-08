#include "geographic_track.h"
#include "protocol_common.h"
#include "protocol_sf.h"
#include <unistd.h>


extern GlobalReatimeMsg* globalMsg;  //全局状态信息  包含相机、激光测距、伺服
extern SF_msg* sf_msg;                     //伺服消息处理对象

GeoGraphicTrack::GeoGraphicTrack()
{

}

GeoGraphicTrack::~GeoGraphicTrack()
{
	if (cfg)
	{
		delete cfg;
		cfg = NULL;
	}

	if (targetLoc)
	{
		delete targetLoc;
		targetLoc = NULL;
	}
}

void GeoGraphicTrack::init()
{
	cfg = new GeoCfg();
	targetLoc = new TargetLoclization();
}

void GeoGraphicTrack::run()
{
	RK_S32 s32Ret = RK_SUCCESS;
	int32_t lastTime = getCurTickCount();
	int32_t frame = 0;
	time_t now;
	int ret = 2;
	while (!cfg->bThreadExit)
	{
		if (cfg->b_geotrack_on)
		{
			SFAngleMode mode = SFAngleMode::AIR_GD;
			double L = globalMsg->m_gnssMsg.d_GPS_Longitude_value;
			double B = globalMsg->m_gnssMsg.d_GPS_Latitude_value;
			double H = globalMsg->m_gnssMsg.d_GPS_Altitude_value;
			double yaw = globalMsg->m_gnssMsg.d_Heading_value;
			double pitch = globalMsg->m_gnssMsg.d_Pitch_value;
			double roll = globalMsg->m_gnssMsg.d_Roll_value;
			
			double sf_yaw = 0;
			double sf_pitch = 0;
			double distance = 0;

			targetLoc->targetPolarFromGps(L, B, H, yaw, pitch, roll, cfg->target_longitude, cfg->target_latitude, cfg->target_altitude,
				sf_yaw, sf_pitch, distance, mode);

			sf_msg->SendFollow(int32_t(sf_yaw * 1000), int32_t(sf_pitch * 1000));

			usleep(10);
		}
	}
}
