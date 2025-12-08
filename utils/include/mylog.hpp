#ifndef _MY_LOG_HPP
#define _MY_LOG_HPP

#include <ctime>
#include <cstring>
#include <string>
#include <iostream>

#include <unistd.h>   //进程管理函数
#include <cstdlib>    //内存分配、程序退出
#include <sys/stat.h> //用于获取文件的状态信息
#include <glog/logging.h>

enum LogLevel
{
    INFO = google::INFO,
    WARNING = google::WARNING,
    ERROR = google::ERROR,
    FATAL = google::FATAL
};

class MyLog
{
public:
    /**
     * @brief 默认构造函数
     * @param savePath 存储路径
     * @param logLevel 日志等级
     **/
    MyLog(const std::string& savePath, LogLevel logLevel);

    /**
     * @brief 重载构造函数
     * @param timestamp 时间戳
     * @param savePath 存储路径
     * @param logLevel 日志等级
     **/
    MyLog(const std::string& timestamp, const std::string& savePath, LogLevel logLevel);

    /**
     * @brief 析构函数
     **/
    ~MyLog();

private:
    std::string m_logPath;           // 存储路径
    const std::string m_logFileName; // 存储文件名字  可能在多个线程里同时记录日志，因此采用const

    /**
     * @brief 获取当前程序的路径
     * @return 当前的路径
     **/
    std::string getCurrentPath();

    /**
     * @brief 根据给定的路径生成日志文件的存储路径
     * @param path 日志路径类型（如 "run" 或 "build"）
     * @return 日志存储路径
     **/
    std::string getLogPath(const std::string& path);

    /**
     * @brief 获取当前时间，并将其格式化为日志文件名
     * @return 格式化后的时间字符串
     **/
    std::string getCurrentTimeForLogFile();

    /**
     * @brief 创建存储路径的多级目录
     * @param dir 目录路径
     **/
    void createMultilevelDir(const char* dir);

    /**
     * @brief 配置日志相关参数
     * @param savePath 存储路径
     * @param logLevel 日志等级
     **/
    void setupLogging(const std::string& savePath, LogLevel logLevel);
};

// 声明全局 MyLog 对象指针
extern MyLog* mylogInstance;

// 初始化 MyLog 实例
void initMyLog(LogLevel logLevel = INFO, const std::string savePath = "", const std::string& timestamp = "");

// 销毁 MyLog 实例
void shutdownMyLog();

#endif // _MY_LOG_HPP