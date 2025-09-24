// include/Utils.h
#pragma once
#include <string>

namespace utils {

    std::string readFile(const std::string &filepath);      // 读取整个文件内容
    std::string getCurrentTime();                            // 获取当前时间字符串
    std::string getFileExtension(const std::string &path);   // 获取文件后缀
    std::string joinPath(const std::string &dir, const std::string &file); // 拼接路径

}
