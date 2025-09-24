// include/Config.h
#pragma once
#include <string>
#include <unordered_map>

class Config {
public:
    Config() = default;
    ~Config() = default;

    // 读取配置文件
    bool load(const std::string &filename);

    // 获取配置值
    std::string getString(const std::string &key, const std::string &defaultVal="") const;
    int getInt(const std::string &key, int defaultVal=0) const;

private:
    std::unordered_map<std::string, std::string> configMap;

    // 辅助函数：去除首尾空格
    static std::string trim(const std::string &s);
};
