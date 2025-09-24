// include/Logger.h
#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <thread>

enum class LogLevel { DEBUG, INFO, ERROR };

class Logger {
public:
    Logger(const std::string &filename);   // 构造函数，指定日志文件路径
    ~Logger();

    void log(LogLevel level, const std::string &msg);

    void debug(const std::string &msg);
    void info(const std::string &msg);
    void error(const std::string &msg);

private:
    std::ofstream logFile;
    std::mutex mtx;

    std::string getTime();                  // 获取当前时间字符串
    std::string levelToString(LogLevel level);
    std::string getThreadId();              // 获取线程 ID
};
