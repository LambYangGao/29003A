#pragma once
#include <vector>
#include<iostream>
#include<math.h>


enum class SFAngleMode {
	LAND_GD,			                        /**< k88002 地面光电：          方位顺时针为正0~360  俯仰上正下负 -90~90 */
	AIR_GD,		                            /**< 5171    机载吊舱：          方位逆时针为正0~360   俯仰上正下负 -90~90*/
};//target localization


class TargetLoclization
{
public:
	TargetLoclization();


	~TargetLoclization();

	//
		/**
	* @brief targetGpsFromPolar 根据目标方位、俯仰和距离，飞机经纬高计算目标经纬高
	* @param yaw:         输入 目标伺服方位 ，单位为度
	* @param pitch：      输入 目标伺服俯仰，单位为度
	* @param distance： 输入 目标距离，     单位米
	* @param b_yaw:     输入 载机导航姿态 方位 ，单位为度
	* @param b_pitch：  输入 载机导航姿态 俯仰 ，单位为度
	* @param b_roll：    输入 载机导航姿态 横滚 ，单位为度
	* @param base_L:    输入 载机经度 ，单位为度
	* @param base_B：  输入 载机维度 ，单位为度
	* @param base_H：  输入 载机高度 ，单位为度
	* @param mode：    输入 吊舱类型
	* @param obj_L： 输出 目标经度，单位为度
	* @param obj_B： 输出 目标维度，单位为度
	* @param obj_H： 输出 目标高度，单位米
	*/
	int targetGpsFromPolar(const double yaw, const double pitch, const double distance, const double b_yaw, const double b_pitch, const double b_roll,
		const double base_L, const double base_B, const double base_H, double& obj_L, double& obj_B, double& obj_H, SFAngleMode mode = SFAngleMode::LAND_GD);

		/**
		* @brief targetPolarFromGps 根据目标经纬高、栽机经纬高、栽机姿态计算目标方位俯仰距离
		* @param base_L:    输入 载机经度 ，单位为度
		* @param base_B：  输入 载机维度 ，单位为度
		* @param base_H：  输入 载机高度 ，单位为度
		* @param obj_L：    输出 目标经度，单位为度
		* @param obj_B：    输出 目标维度，单位为度
		* @param obj_H：   输出 目标高度，单位米
		* @param b_yaw:     输入 载机导航姿态 方位 ，单位为度
		* @param b_pitch：  输入 载机导航姿态 俯仰 ，单位为度
		* @param b_roll：    输入 载机导航姿态 横滚 ，单位为度
		* @param mode：    输入 吊舱类型
		* @param yaw:        输出 目标伺服方位 ，单位为度
		* @param pitch：     输出 目标伺服俯仰， 单位为度
		* @param distance：输出 目标距离，       单位米
	*/
	int targetPolarFromGps(const double base_L, const double base_B, const double base_H, const double b_yaw, const double b_pitch, const double b_roll,
		const double obj_L, const double obj_B, double const obj_H, double& yaw, double& pitch, double& distance, SFAngleMode mode = SFAngleMode::LAND_GD);

		/**
		* @brief calcGpsDistance 根据目标经纬高、栽机经纬高 计算二者之间距离
		* @param base_L:    输入 载机经度 ，单位为度
		* @param base_B：  输入 载机维度 ，单位为度
		* @param base_H：  输入 载机高度 ，单位为度
		* @param obj_L：    输入 目标经度，单位为度
		* @param obj_B：    输入 目标维度，单位为度
		* @param obj_H：    输入 目标高度，单位米
	*/
	double calcGpsDistance(double src_L, double src_B, double src_H, double target_L, double target_B, double target_H);

		/**
		* @brief calcBody2EnuMat 根据载机姿态计算载体坐标系到东北天坐标系的转换（默认以光电为站心）
		* @param yaw:     输入 载机航向，单位为度
		* @param pitch：  输入 载机俯仰 ，单位为度
		* @param roll：    输入 载机横滚，单位为度
		* @param body_x：  输入 载机横滚，单位为度
		* @param body_y：  输入 载机横滚，单位为度
		* @param body_z：  输入 载机横滚，单位为度
		* @param e：  输出  东向坐标
		* @param n：  输出 北向坐标，单位为度
		* @param u：  输出 天向坐标，单位为度
	*/
	void calcBody2Enu(const double yaw, const double pitch, const double roll,
		const double body_x, const double body_y, const double body_z, double& e, double& n, double& u);

	/**
		* @brief calcEnu2EcefMat 根据载机当前经纬高计（ 并以当前光电为站心）载机东北天坐标系到ECEF坐标系转换矩阵
		* @param longitude:    输入 载机经度，单位为度 站心经度
		* @param latitude：  输入 载机维度，单位为度    站心维度
		* @param altitude：  输入 载机高度，单位为度    站心高度
		* @param e：  输入 载机enu坐标 东向坐标
		* @param n：  输入 载机enu坐标 北向坐标
		* @param u：  输入 载机enu坐标 天向坐标
		* @param ecef_x：  输出大地坐标x
		* @param ecef_y：  输出大地坐标y
		* @param ecef_z： 输出大地坐标z
	*/
	void calcEnu2Ecef(const double longitude, const double latitude, const double altitude,
		const double e, const double n, const double u, double& ecef_x, double& ecef_y, double& ecef_z);

	/**
		  * @brief ecef2WGS84 ECEF转WGS-84  XYZ转经纬高
		  * @param x： 输入 ECEF大地坐标X ，单位米
		  * @param y： 输入 ECEF大地坐标Y，单位米
		  * @param z： 输入 ECEF大地坐标Z，单位米
		  * @param longitude: 输出经度 ，单位为度
		  * @param latitude：  输出纬度，单位为度
		  * @param altitude： 输出高度，单位米
	  */
	void ecef2WGS84(const double x, const double y, const double z, double& longitude, double& latitude, double& altitude);

	/**
		* @brief wgs842Ecef WGS-84转ECEF  经纬高转ecef大地坐标系
		* @param longitude: 输入经度 ，单位为度
		* @param latitude： 输入纬度，单位为度
		* @param altitude： 输入高度，单位米
		* @param x： 输出 ECEF大地坐标X ，单位米
		* @param y： 输出 ECEF大地坐标Y，单位米
		* @param z： 输出 ECEF大地坐标Z，单位米
	*/
	void wgs842ECEF(const double longitude, const double latitude, const double altitude,
		double& x, double& y, double& z);

private:
	class LocImpl;
	LocImpl* locImpl=nullptr;
};