#ifndef GEOGRAPHIC_TRACK_H
#define GEOGRAPHIC_TRACK_H
#include "targetlocation.h"
#include "rk_type.h"
#include "rk_debug.h"
#include "ConfigManager.h"
class GeoGraphicTrack
{
public:
	GeoGraphicTrack();
	~GeoGraphicTrack();

	struct GeoCfg
	{
		RK_BOOL bThreadExit = RK_FALSE;//线程退出标志
		RK_BOOL b_geotrack_on = RK_FALSE;
		
		double target_latitude;    //纬度
		double target_longitude;  //经度
		double target_altitude;    //高度

		void setTargetLBH(double longitude, double latitude, double altitude)
		{
			target_latitude = latitude;
			target_longitude = longitude;
			target_altitude = altitude;
		}
	};
	GeoCfg* cfg = NULL;
	void init();//初始化

	void run();//线程函数

private:
	//地理跟踪
	TargetLoclization* targetLoc = nullptr;
};





#endif