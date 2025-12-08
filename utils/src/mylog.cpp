#include "mylog.hpp"

MyLog* mylogInstance = nullptr;

// 初始化日志系统
void initMyLog(LogLevel logLevel, const std::string savePath, const std::string& timestamp)
{
    if (mylogInstance == nullptr)
    {
        if (timestamp.empty())
            mylogInstance = new MyLog(savePath, logLevel); // 使用默认构造函数
        else
            mylogInstance = new MyLog(timestamp, savePath, logLevel); // 使用带timestamp的构造函数
    }
}

// 关闭日志系统
void shutdownMyLog()
{
    if (mylogInstance != nullptr)
    {
        delete mylogInstance;
        mylogInstance = nullptr;
    }
}

// MyLog 构造函数：不带 timestamp
MyLog::MyLog(const std::string& savePath, LogLevel logLevel) : m_logFileName(getCurrentTimeForLogFile())
{
    setupLogging(savePath, logLevel);
}

// MyLog 构造函数：带 timestamp
MyLog::MyLog(const std::string& timestamp, const std::string& savePath, LogLevel logLevel) : m_logFileName(timestamp)
{
    setupLogging(savePath, logLevel);
}

// 析构函数
MyLog::~MyLog()
{
    google::ShutdownGoogleLogging();
}

// 设置日志相关配置
void MyLog::setupLogging(const std::string& savePath, LogLevel logLevel)
{
    FLAGS_alsologtostderr = true;           // 设置日志消息除了日志文件之外还可以显示到终端上
    FLAGS_colorlogtostderr = true;          // 设置控制台日志颜色，区分不同日志级别
    FLAGS_log_prefix = true;                // 设置日志前缀（日志时间、等级、线程ID、文件名等）
    FLAGS_logbufsecs = 0;                   // 日志实时写入文件、不进行缓存
    FLAGS_max_log_size = 20;                // 单个日志文件最大大小 (单位：MB)，超过会对文件进行分割
    FLAGS_stop_logging_if_full_disk = true; // 磁盘空间不足，停止写入日志，避免系统因为磁盘空间耗尽而崩溃。
    FLAGS_minloglevel = logLevel;           // 使用传入的 logLevel 设置最低日志等级

    m_logPath = getLogPath(savePath);
    if (m_logPath.empty())
    {
        std::cerr << "Error: log path is empty!" << std::endl;
        return;
    }

    google::InitGoogleLogging("LogInfo");

    // 根据日志级别动态设置日志文件前缀
    switch (logLevel)
    {
    case LogLevel::INFO:
        google::SetLogDestination(google::INFO, (m_logPath + "/INFO_").c_str());
        break;
    case LogLevel::WARNING:
        google::SetLogDestination(google::WARNING, (m_logPath + "/WARNING_").c_str());
        break;
    case LogLevel::ERROR:
        google::SetLogDestination(google::ERROR, (m_logPath + "/ERROR_").c_str());
        break;
    case LogLevel::FATAL:
        google::SetLogDestination(google::FATAL, (m_logPath + "/FATAL_").c_str());
        break;
    default:
        break;
    }
}

// 创建多级目录
void MyLog::createMultilevelDir(const char* dir)
{
    char command[256];
    snprintf(command, sizeof(command), "mkdir -p %s", dir);
    system(command);
}

// 获取当前路径
std::string MyLog::getCurrentPath()
{
    char* currentPath = getcwd(NULL, 0); // 获取当前工作目录
    if (currentPath == NULL)
    {
        std::cerr << "Error getting current path" << std::endl;
    }

    std::string strCurrentPath(currentPath); // 使用 currentPath 创建 std::string
    free(currentPath);                       // 释放内存
    return strCurrentPath;                   // 返回当前路径
}

// 获取日志保存路径
std::string MyLog::getLogPath(const std::string& path)
{
    std::string currentPath = getCurrentPath();
    if (currentPath.empty())
    {
        std::cerr << "Error: failed to get current path!" << std::endl;
    }

    std::string logPath;
    if (path == "build")
    {
        int index = currentPath.find_last_of('/');
        if (index == std::string::npos)
        {
            std::cerr << "Error: failed to find last '/' in path!" << std::endl;
        }
        logPath = currentPath.substr(0, index) + "/logs/" + m_logFileName;
    }
    else if (path == "run")
    {
        logPath = currentPath + "/logs/" + m_logFileName;
    }
    else
    {
        logPath = path + "/" + m_logFileName;
    }

    if (access(logPath.c_str(), F_OK) != 0) // 检查路径是否存在
    {
        createMultilevelDir(logPath.c_str());
    }

    return logPath;
}

// 获取当前时间用于日志文件名
std::string MyLog::getCurrentTimeForLogFile()
{
    time_t currentTime = std::time(nullptr); // 获取当前时间
    char buffer[80];

    // 使用strftime格式化时间，将其格式化为YYYYMMDD_HHMMSS
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", localtime(&currentTime));

    return std::string(buffer); // 返回格式化后的字符串
}