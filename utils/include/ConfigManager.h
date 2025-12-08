#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string.h>

/// <summary>
/// 下位机配置文件信息
/// </summary>
typedef struct GDConfigInfo
{
	//系统参数
	char GD_A_IP[15];//综合处理板3588A IP：10.5.226.11
	char GD_B_IP[15];//综合处理板3588B IP：10.5.226.12
	char GD_C_IP[15];//综合处理板3588B IP：10.5.226.13
	char MNXK_IP[15];//显控模拟器IP：10.5.226.150

	int GD_Send_MNXK_Port;//发送FC状态信息至显控模拟器 for 监测：11000
	int A_Send_B_Port;//综合处理板3588A 发送至 3588B端口
	int B_Send_A_Port;//综合处理板3588B 发送至 3588A端口
	int A_Send_C_Port;//综合处理板3588A 发送至 ATLAS200端口

	int DeviceNum;//样机编号 1号机凯普林   2号机亮源

	//校靶参数
	int32_t SF_fwBias;//载机0位与伺服0位的方位偏差  +180000～-180000
	int32_t SF_fyBias;//载机0位与伺服0位俯仰偏差  +30000～-135000

	int32_t SF_pybcFwNum;//漂移补偿方位值 -5000~5000 步进100
	int32_t SF_pybcFyNum;//漂移补偿俯仰值 -5000~5000 步进100

	//软校轴参数
	float TRACK_VIB_X_Bias;
	float TRACK_VIB_Y_Bias;	
	float TRACK_VI_X_Bias;
	float TRACK_VI_Y_Bias;
	float TRACK_IRZ_X_Bias;
	float TRACK_IRZ_Y_Bias;
	float TRACK_IRZB_X_Bias;
	float	TRACK_IRZB_Y_Bias;
	float TRACK_IRD_X_Bias;
	float TRACK_IRD_Y_Bias;

	//安装误差
	float InstallDevi_azi_value;//安装误差方位角 -20~20 左负右正
	float InstallDevi_pitch_value;//安装误差俯仰角 -20~20 下负上正
	float InstallDevi_roll_value;//安装误差横滚角 -20~20 逆负顺正

	float INSDevi_azi_value;//修正惯导航向误差 -3~3 分辨率0.0001
	float INSDevi_pitch_value;//修正惯导俯仰误差 -3~3 分辨率0.0001
	float INSDevi_roll_value;//修正惯导横滚误差 -3~3 分辨率0.0001

	float INSDevi_Longitude_value;//修正惯导经度误差 -3~3 分辨率0.0001
	float INSDevi_Latitude_value;//修正惯导纬度误差 -0.003~0.003 分辨率0.0000001
	float INSDevi_Altitude_value;//修正惯导高度误差 -300~300 分辨率0.01

	float InstallDevi_X_value;//安装误差X轴 -20～+20m，以飞机质心为原点，沿航向方向为正
	float InstallDevi_Y_value;//安装误差Y轴 -20～+20m，以飞机质心为原点，下负，上正
	float InstallDevi_Z_value;//安装误差Y轴 -20～+20m，以飞机质心为原点，左机翼负，右机翼正
};

class ConfigManager
{
private:
	std::string string_replace(std::string source, std::string find, std::string replace);
	//用于支持配置文件中的字符串转数字
	template <class T>
	void convertFromString(T& value, const std::string& s) {
		std::stringstream ss(s);
		ss.precision(s.length());
		ss >> value;
	}
	//用于去除字符串多余的空格
	std::string trim(std::string text);
	//用于支持将多行的配置文件分割开来
	void Stringsplit(std::string str, const char split, std::vector<std::string>& strList);
	//用于支持将 key=value 格式的配置文件分割开来（只分割一次）
	void Stringsplit2(std::string str, const char split, std::vector<std::string>& strList);

public:
	GDConfigInfo* m_gd_configinfo;//配置信息
	void init_GDConfigInfo();

	std::string CharToStr(char* conttentChar);
	bool rewrite(std::string& filename, std::vector<std::string>& lineDatas);
	bool modifyContentInfile(std::string filename, std::string key, std::string content);
	ConfigManager() {
		m_gd_configinfo = new GDConfigInfo;
	}
	ConfigManager(std::string fname) {
		m_gd_configinfo = new GDConfigInfo;
		parse_file(fname);
		if (read("print_cfg", 0) > 0) {//print_cfg>0时输出配置文件
			print();
		}
	}
	std::map<std::string, std::string> maps;
	//解析配置文件，并添加默认配置
	void parse_file(std::string fname) {
		std::string strdata;
		try {
			std::ifstream in(fname, std::ios::in);
			std::istreambuf_iterator<char> beg(in), end;
			strdata = std::string(beg, end);
			in.close();
			if (strdata.length() < 10) {
				std::cout << fname << " context is not correct! " << std::endl;
			}
		}
		catch (...) {
			std::cout << "Read " << fname << " error! " << std::endl;
		}

		std::vector<std::string> strList;
		Stringsplit(strdata, '\n', strList);
		//std::map<std::string, std::string> maps;
		for (int i = 0; i < strList.size(); i++) {
			std::vector<std::string> tmp1, tmp2;
			if (strList[i].length() >= 1) {
				Stringsplit2(strList[i], '#', tmp1);//用于清除注释  注释存储在trim(tmp1[1])中
				if (tmp1.size() >= 1) {
					Stringsplit2(tmp1[0], '=', tmp2);//把字符串分为两节
					if (tmp2.size() > 1) {
						maps.insert({ trim(tmp2[0]),trim(tmp2[1]) });//去除字符串多余的空格（包含 \n\r\t）
					}
				}
			}
		}
		return;
	}
	//用于输出配置文件，或保存配置文件
	void print(std::string file_name = "") {
		std::map<std::string, std::string>::iterator iter1;
		std::string str = "";

		for (iter1 = maps.begin(); iter1 != maps.end(); iter1++)
		{
			str += iter1->first + "=" + iter1->second + "\n";
		}
		if (file_name.length() == 0) {
			std::cout << str;
		}
		else {
			//// 向文件里写数据
			//std::ofstream os;     //创建一个文件输出流对象
			//// os.open("../books.txt", std::fstream::out|std::fstream::app);//将对象与文件关联, app就是每次在文件后面继续添加内容
			//os.open(file_name, std::fstream::out);//将对象与文件关联
			//os << str;   //将输入的内容放入txt文件中
			//os.close();
			FILE* pf = fopen(file_name.c_str(), "wb");
			if (pf == NULL)
			{
				printf("file open error!\n");
				return;
			}
			fwrite(str.c_str(), str.length(), 1, pf);
			fclose(pf);
			pf = NULL;
		}
	}

	//用于安全读取配置文件（只用config.find(key)->second写错了key会导致报错）
	std::string read(std::string key, std::string defaultv = "") {
		auto it = maps.find(key);
		if (it == maps.end()) {
			return defaultv;
		}
		else {
			return it->second;
		}
	}
	//用于安全读取配置文件 int类型（只用config.find(key)->second写错了key会导致报错）
	int read(std::string key, int defaultv = 0) {
		auto it = maps.find(key);
		if (it == maps.end()) {
			return defaultv;
		}
		else {
			std::string s = it->second;
			int res;
			convertFromString(res, s);
			return res;
		}
	}
	//用于安全读取配置文件 float类型（只用config.find(key)->second写错了key会导致报错）
	//下位机使用报段错误
	float read(std::string key, float defaultv = 0) {
		auto it = maps.find(key);
		if (it == maps.end()) {
			return defaultv;
		}
		else {
			std::string s = it->second;
			float res;
			convertFromString(res, s);
			printf("%f\n", res);
			return res;
		}
	}
	//用于配置文件赋值，当key不存在时为新增
	void set(std::string key, std::string value) {
		maps[key] = value;
		//printf(maps[key].c_str());
		//printf("\n");
		//printf(value.c_str());
		//printf("\n");
	}
};

/*****************************************************************************
* function: 设置某个文件中 xxx = yyy
* cfgFileName: in, 文件名
* optName: in, xxx
* optValue: in, yyy
* 返回: 成功0 失败-1
*******************************************************************************/
int SetDevCfg(const char* cfgFileName, const char* optName, const char* optValue, const int optValue_len);

/*****************************************************************************
* function: 获取某个文件中 xxx = yyy 的yyy
* cfgFileName: in, 文件名
* optName: in, xxx
* optValue: out, yyy
* 返回: yyy的长度，失败返回0
*******************************************************************************/
int GetDevCfg(const char* sFileName, const char* optName, char* optValue, int& optValue_len);