#include "ConfigManager.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <rk_debug.h>
#define BUFFER_SIZE				4096
//-------------辅助函数-------------
std::string ConfigManager::string_replace(std::string source, std::string find, std::string replace)
{
	std::string::size_type pos = 0;
	std::string::size_type a = find.size();
	std::string::size_type b = replace.size();
	while ((pos = source.find(find, pos)) != std::string::npos)
	{
		//source.replace(pos, a, replace);
		source.erase(pos, a);
		source.insert(pos, replace);
		pos += b;
	}
	return source;
}
////用于支持配置文件中的字符串转数字
//template <class T>
//void ConfigManager::convertFromString(T& value, const std::string& s) {
//	std::stringstream ss(s);
//	ss.precision(s.length());
//	ss >> value;
//}

//用于去除字符串多余的空格
std::string ConfigManager::trim(std::string text)
{
	if (!text.empty())
	{
		text.erase(0, text.find_first_not_of(" \n\r\t"));//去除字符串头部的空格
		text.erase(text.find_last_not_of(" \n\r\t") + 1);//去除字符串尾部的空格
	}
	return text;
}
//用于支持将多行的配置文件分割开来
void ConfigManager::Stringsplit(std::string str, const char split, std::vector<std::string>& strList)
{
	std::istringstream iss(str);	// 输入流
	std::string token;			// 接收缓冲区
	while (getline(iss, token, split))	// 以split为分隔符
	{
		strList.push_back(token);
	}
}
//用于支持将 key=value 格式的配置文件分割开来（只分割一次）
void ConfigManager::Stringsplit2(std::string str, const char split, std::vector<std::string>& strList)
{
	//string str = "key=value1 value2 #notes";
	size_t pos = str.find(split); // 3
	if (pos > 0 && pos < str.length()) {//用于分割key=value
		std::string p = str.substr(0, pos); // 123
		std::string q = str.substr(pos + 1); // 456,789
		strList.push_back(p);
		strList.push_back(q);
	}
	else {//用于不带# 注释时的分割
		strList.push_back(str);
	}
}

void ConfigManager::init_GDConfigInfo()
{
	print();

	strcpy(m_gd_configinfo->GD_A_IP, read("GD_A_IP", "").c_str());//综合处理板3588A IP：10.5.226.11
	strcpy(m_gd_configinfo->GD_B_IP, read("GD_B_IP", "").c_str());//综合处理板3588B IP：10.5.226.12
	strcpy(m_gd_configinfo->GD_C_IP, read("GD_C_IP", "").c_str());//综合处理板3588B IP：10.5.226.13
	strcpy(m_gd_configinfo->MNXK_IP, read("MNXK_IP", "").c_str());//显控模拟器IP：10.5.226.150

	m_gd_configinfo->GD_Send_MNXK_Port = read("GD_Send_MNXK_Port", 0);//发送FC状态信息至显控模拟器 for 监测：11000
	m_gd_configinfo->A_Send_B_Port = read("A_Send_B_Port", 0);//综合处理板3588A 发送至 3588B端口
	m_gd_configinfo->B_Send_A_Port = read("B_Send_A_Port", 0);//综合处理板3588B 发送至 3588A端口
	m_gd_configinfo->A_Send_C_Port = read("A_Send_C_Port", 0);//综合处理板3588B 发送至 3588A端口

	m_gd_configinfo->DeviceNum = read("DeviceNum", 0);//样机编号 1 2

	//校靶参数
	m_gd_configinfo->SF_fwBias = read("SF_fwBias", 0);//载机0位与伺服0位的方位偏差  +180000～-180000
	m_gd_configinfo->SF_fyBias = read("SF_fyBias", 0);//载机0位与伺服0位俯仰偏差  +30000～-135000
	m_gd_configinfo->SF_pybcFwNum = read("SF_pybcFwNum", 0);
	m_gd_configinfo->SF_pybcFyNum = read("SF_pybcFyNum", 0);

	//软校轴参数
	m_gd_configinfo->TRACK_VI_X_Bias = read("TRACK_VI_X_Bias", 0.0F);
	m_gd_configinfo->TRACK_VI_Y_Bias = read("TRACK_VI_Y_Bias", 0.0F);	
	m_gd_configinfo->TRACK_VIB_X_Bias = read("TRACK_VIB_X_Bias", 0.0F);
	m_gd_configinfo->TRACK_VIB_Y_Bias = read("TRACK_VIB_Y_Bias", 0.0F);
	m_gd_configinfo->TRACK_IRZ_X_Bias = read("TRACK_IRZ_X_Bias", 0.0F);
	m_gd_configinfo->TRACK_IRZ_Y_Bias = read("TRACK_IRZ_Y_Bias", 0.0F);	
	m_gd_configinfo->TRACK_IRZB_X_Bias = read("TRACK_IRZB_X_Bias", 0.0F);
	m_gd_configinfo->TRACK_IRZB_Y_Bias = read("TRACK_IRZB_Y_Bias", 0.0F);
	m_gd_configinfo->TRACK_IRD_X_Bias = read("TRACK_IRD_X_Bias", 0.0F);
	m_gd_configinfo->TRACK_IRD_Y_Bias = read("TRACK_IRD_Y_Bias", 0.0F);
	//安装误差
	m_gd_configinfo->InstallDevi_azi_value = read("InstallDevi_azi_value", 0.0F);//安装误差方位角 -20~20 左负右正
	m_gd_configinfo->InstallDevi_pitch_value = read("InstallDevi_pitch_value", 0.0F);//安装误差俯仰角 -20~20 下负上正
	m_gd_configinfo->InstallDevi_roll_value = read("InstallDevi_roll_value", 0.0F);//安装误差横滚角 -20~20 逆负顺正

	m_gd_configinfo->INSDevi_azi_value = read("INSDevi_azi_value", 0.0F);//修正惯导航向误差 -3~3 分辨率0.0001
	m_gd_configinfo->INSDevi_pitch_value = read("INSDevi_pitch_value", 0.0F);//修正惯导俯仰误差 -3~3 分辨率0.0001
	m_gd_configinfo->INSDevi_roll_value = read("INSDevi_roll_value", 0.0F);//修正惯导横滚误差 -3~3 分辨率0.0001

	m_gd_configinfo->INSDevi_Longitude_value = read("INSDevi_Longitude_value", 0.0F);//修正惯导经度误差 -3~3 分辨率0.0001
	m_gd_configinfo->INSDevi_Latitude_value = read("INSDevi_Latitude_value", 0.0F);//修正惯导纬度误差 -0.003~0.003 分辨率0.0000001
	m_gd_configinfo->INSDevi_Altitude_value = read("INSDevi_Altitude_value", 0.0F);//修正惯导高度误差 -300~300 分辨率0.01

	m_gd_configinfo->InstallDevi_X_value = read("InstallDevi_X_value", 0.0F);//安装误差X轴 -20～+20m，以飞机质心为原点，沿航向方向为正
	m_gd_configinfo->InstallDevi_Y_value = read("InstallDevi_Y_value", 0.0F);//安装误差Y轴 -20～+20m，以飞机质心为原点，下负，上正
	m_gd_configinfo->InstallDevi_Z_value = read("InstallDevi_Z_value", 0.0F);//安装误差Y轴 -20～+20m，以飞机质心为原点，左机翼负，右机翼正
}

std::string ConfigManager::CharToStr(char* conttentChar)
{
	std::string tempStr = "";
	for (int i = 0; conttentChar[i] != '\0'; i++)
	{
		tempStr += conttentChar[i];
	}
	return tempStr;
}

bool ConfigManager::rewrite(std::string& filename, std::vector<std::string>& lineDatas)
{
	FILE* pf = fopen(filename.c_str(), "wb");
	if (pf == NULL)
	{
		printf("file open error!\n");
		return false;
	}
	for (const auto& str : lineDatas)
	{
		fwrite(str.c_str(), str.length(), 1, pf);
		fwrite("\n",1,1,pf);
	}
	fclose(pf);
	pf = NULL;
	return true;
	//std::fstream file(filename, std::ios::out);

	//if (!file.is_open())
	//{
	//	std::cerr << filename << " open failed." << std::endl;
	//	return false;
	//}
	//for (const auto& str : lineDatas)
	//{
	//	file << str << std::endl;
	//}

	//file.close();

	//std::ofstream os;     //创建一个文件输出流对象
	//        // os.open("../books.txt", std::fstream::out|std::fstream::app);//将对象与文件关联, app就是每次在文件后面继续添加内容
	//os.open(filename, std::fstream::out);//将对象与文件关联
	//for (const auto& str : lineDatas)
	//{
	//    os << str << std::endl;
	//}
	//os.close();

}

bool ConfigManager::modifyContentInfile(std::string filename, std::string key, std::string content)
{
	if (!filename.empty())
	{
		std::fstream file(filename, std::ios::in);

		if (!file.is_open())
		{
			std::cerr << filename << " open failed." << std::endl;
			return false;
		}

		char buffer[1024]{ 0 };
		std::string ikey = "#";

		std::vector<std::string> lineDatas;
		while (!file.eof())
		{
			file.getline(buffer, 1024);
			if (CharToStr(buffer).find(key) != std::string::npos && CharToStr(buffer).find(ikey) == std::string::npos)
			{
				char szLine[1024] = { 0 };
				strcpy(szLine, content.c_str());
				lineDatas.push_back(szLine);

				continue;
			}

			lineDatas.push_back(buffer);
		}
		///< 回写
		if (rewrite(filename, lineDatas))
		{
			std::cerr << filename << " rewrite success." << std::endl;
		}
		else
		{
			std::cerr << filename << " rewrite  failed." << std::endl;
		}

		file.close();
	}
	//char buffer[64] = { 0 };
	//sprintf(buffer, "sed -i 's/%s.*/defaultmode=%dx%d-16bpp/g' %s", key.c_str(), g_stSize.u32Width, g_stSize.u32Height, filename.c_str());
	//system(buffer);
	return true;
}

int SetDevCfg(const char* cfgFileName, const char* optName, const char* optValue, const int optValue_len)
{
	if (!cfgFileName || !optName || !optValue)
	{
		printf("error: [SetNetConfigOption] !cfgFileName || !optName || !optValue\n");
		return -1;
	}
	//文件存在 且可读可写
	if (access(cfgFileName, F_OK | R_OK | W_OK) == -1)
	{
		printf("error: [SetNetConfigOption] file %s invalid !\n", cfgFileName);
		return -1;
	}

	FILE* fd = fopen(cfgFileName, "r+");
	if (!fd)
	{
		printf("error: [SetNetConfigOption] open %s failed !\n", cfgFileName);
		return -1;
	}
	fseek(fd, 0, SEEK_END);
	int file_length = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	char file_buff[BUFFER_SIZE] = { 0 };
	char item_buff[64] = { 0 };
	int length = fread(file_buff, 1, file_length, fd);
	if (length != file_length)
	{
		fclose(fd);
		printf("error: [SetNetConfigOption] %s too large !\n", cfgFileName);
		return -1;
	}


	char* opt_pos = strstr(file_buff, optName);
	if (!opt_pos)
	{
		printf("not find opt_pos\n");
		printf("add end\n");
		memset(item_buff, 0, BUFFER_SIZE);
		memcpy(item_buff, optName, strlen(optName));
		memcpy(item_buff + strlen(optName), "=", 1);
		memcpy(item_buff + strlen(optName) + 1, optValue, optValue_len);
		fwrite(item_buff, 1, strlen(optName) + 1 + optValue_len, fd);
	}
	else
	{
		printf("find opt_pos\n");
		ftruncate(fileno(fd), 0);
		fseek(fd, 0, SEEK_SET);
		fwrite(file_buff, 1, opt_pos - file_buff, fd);
		memset(item_buff, 0, BUFFER_SIZE);
		memcpy(item_buff, optName, strlen(optName));
		memcpy(item_buff + strlen(optName), "=", 1);
		memcpy(item_buff + strlen(optName) + 1, optValue, optValue_len);
		fwrite(item_buff, 1, strlen(optName) + 1 + optValue_len, fd);
		char* last_content = strchr(opt_pos, '\n');
		if (last_content)
		{
			fwrite(last_content, 1, file_length - (last_content - file_buff), fd);
		}
	}
	fclose(fd);
	return 0;
}

int GetDevCfg(const char* sFileName, const char* optName, char* optValue, int& optValue_len)
{
	int ret = -1;
	if (!sFileName || !optName || !optValue)
	{
		printf("error: [GetConfigOption] !sFileName || !sKey || !sValue\n");
		return ret;
	}
	//文件存在 且可读可写
	if (access(sFileName, F_OK | R_OK | W_OK) == -1)
	{
		printf("error: [SetNetConfigOption] file %s invalid !\n", sFileName);
		return ret;
	}

	FILE* fd = fopen(sFileName, "r");
	if (!fd)
	{
		printf("error: [SetNetConfigOption] open %s failed !\n", sFileName);
		return ret;
	}
	fseek(fd, 0, SEEK_END);
	int file_length = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	char buffer[BUFFER_SIZE] = { 0 };
	int length = fread(buffer, 1, file_length, fd);
	if (length != file_length)
	{
		printf("error: [SetNetConfigOption] %s too large !\n", sFileName);
		fclose(fd);
		return ret;
	}

	char* pos = strstr(buffer, optName);

	//查找到的sKey没有越过下一个标签
	if (pos)
	{
		char* ptr = pos + strlen(optName);
		//循环不能越过文件长度
		while (ptr - buffer < file_length)
		{
			if (*ptr == '=')
			{
				//找到了
				break;
			}
			else if (*ptr == ' ')
			{
				//空格跳过
				ptr++;
				continue;
			}
			else
			{
				//不是空格 也不是等号 说明有其它字符
				ptr = NULL;
				ret = -1;
				break;
			}
		}
		if (ptr && *ptr == '=')
		{
			//去掉空格和等号
			while ((ptr[0] == ' ' || ptr[0] == '=') && ptr < buffer + file_length)
			{
				ptr++;
			}
			for (int i = 0; *ptr != '\n' && ptr < buffer + file_length; i++, ptr++)
			{
				optValue[i] = *ptr;
				optValue_len = i + 1;
			}
			ret = 0;
		}
	}
	fclose(fd);
	return ret;
}
