// src/Logger.cpp
#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string &filename) {
    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

std::string Logger::getTime() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return std::string(buf);
}

std::string Logger::getThreadId() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch(level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string &msg) {
    std::lock_guard<std::mutex> lock(mtx);

    std::string output = "[" + getTime() + "][" + levelToString(level) + "][Thread " + getThreadId() + "] " + msg;

    // 打印到控制台
    std::cout << output << std::endl;

    // 写入日志文件
    if (logFile.is_open()) {
        logFile << output << std::endl;
    }
}

// 便捷函数
void Logger::debug(const std::string &msg) { log(LogLevel::DEBUG, msg); }
void Logger::info(const std::string &msg)  { log(LogLevel::INFO, msg); }
void Logger::error(const std::string &msg) { log(LogLevel::ERROR, msg); }
