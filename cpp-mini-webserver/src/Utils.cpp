// src/Utils.cpp
#include "Utils.h"
#include <fstream>
#include <sstream>
#include <ctime>

namespace utils {

std::string readFile(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string getCurrentTime() {
    std::time_t t = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

std::string getFileExtension(const std::string &path) {
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos) return "";
    return path.substr(pos + 1);
}

std::string joinPath(const std::string &dir, const std::string &file) {
    if (dir.back() == '/' || dir.back() == '\\')
        return dir + file;
    else
        return dir + "/" + file;
}

}
