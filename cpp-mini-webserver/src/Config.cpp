// src/Config.cpp
#include "Config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

bool Config::load(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        // 忽略空行和注释
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (!key.empty()) {
            configMap[key] = value;
        }
    }

    return true;
}

std::string Config::getString(const std::string &key, const std::string &defaultVal) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) return it->second;
    return defaultVal;
}

int Config::getInt(const std::string &key, int defaultVal) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultVal;
        }
    }
    return defaultVal;
}

std::string Config::trim(const std::string &s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) ++start;

    auto end = s.end();
    do { --end; } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}
